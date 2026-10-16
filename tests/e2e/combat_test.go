//go:build e2e

// SPDX-License-Identifier: GPL-2.0-or-later
package digimon_test

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"testing"
	"time"

	"github.com/azerothcore/AzerothGhost/client"
	"github.com/azerothcore/AzerothGhost/e2e/e2eharness"
)

func packedGUID(reader *bytes.Reader) (uint64, error) {
	mask, err := reader.ReadByte()
	if err != nil {
		return 0, err
	}
	var guid uint64
	for i := uint(0); i < 8; i++ {
		if mask&(1<<i) == 0 {
			continue
		}
		next, err := reader.ReadByte()
		if err != nil {
			return 0, err
		}
		guid |= uint64(next) << (8 * i)
	}
	return guid, nil
}

func assertHumanSwingsBlocked(t *testing.T, bot *e2eharness.ScenarioBot, target uint64) {
	t.Helper()
	observed := make(chan error, 1)
	hits := 0
	cancel := bot.World.AddPacketHook(func(op uint16, data []byte) {
		if op != client.SmsgAttackerStateUpdate {
			return
		}
		reader := bytes.NewReader(data)
		var info uint32
		if binary.Read(reader, binary.LittleEndian, &info) != nil {
			return
		}
		attacker, err := packedGUID(reader)
		if err != nil {
			return
		}
		victim, err := packedGUID(reader)
		if err != nil {
			return
		}
		var damage uint32
		if binary.Read(reader, binary.LittleEndian, &damage) != nil {
			return
		}
		if attacker != bot.World.CharGUID() || victim != target {
			return
		}
		if damage > 0 {
			select {
			case observed <- fmt.Errorf("human dealt %d melee damage", damage):
			default:
			}
			return
		}
		// HITINFO_MISS is 0x10. At least three non-miss attack results are required.
		if info&0x10 == 0 {
			hits++
		}
		if hits >= 3 {
			select {
			case observed <- nil:
			default:
			}
		}
	})
	defer cancel()
	bot.Attack(t, target)
	select {
	case err := <-observed:
		if err != nil {
			t.Fatal(err)
		}
	case <-time.After(20 * time.Second):
		t.Fatal("could not observe three non-miss human swings")
	}
	if err := bot.World.AttackStop(); err != nil {
		t.Fatal(err)
	}
}

func TestTamer_OnlyCompanionDamagesAndRewardsOwner(t *testing.T) {
	bots := e2eharness.NewScenario(t, e2eharness.ScenarioOpts{Prefix: "DigiC", Bots: []e2eharness.BotSpec{
		{Role: "warrior", Race: e2eharness.RaceHuman, Class: e2eharness.ClassWarrior, Level: 1},
		{Role: "mage", Race: e2eharness.RaceHuman, Class: e2eharness.ClassMage, Level: 1},
	}})
	e2eharness.TeleportAllPad(t, bots, e2eharness.PackagePad(t))
	warrior, mage := bots[0], bots[1]
	guide := warrior.Spawn(t, 900100, 10*time.Second)
	warrior.AddQuest(t, 7) // Verified stock objective: eight Kobold Vermin (entry 6).
	target := warrior.Spawn(t, 6, 10*time.Second)
	// SpawnPersistent explicitly enables GM mode. Finish all fixture spawning
	// before restoring normal faction/combat and selecting the companion.
	warrior.CombatReady(t)
	mage.CombatReady(t)
	selectOption(t, warrior, hello(t, warrior, guide), "Elegir Lobo digital")
	// Prepare the XP boundary on this disposable test account. A real kill must
	// then level the companion above its level-one owner without a core pet cap.
	if _, err := warrior.CharDB.Exec("UPDATE digimon_collection SET experience=174 WHERE owner_guid=?", uint32(warrior.World.CharGUID())); err != nil {
		t.Fatal(err)
	}
	warrior.Relog(t)
	warrior.CombatReady(t)
	companion := waitOwned(t, warrior, 1)[0]
	hp, max := warrior.WaitUnitHPKnown(t, target, 5*time.Second)
	if hp == 0 || max == 0 {
		t.Fatal("target health is unknown")
	}
	assertHumanSwingsBlocked(t, warrior, target)
	result, err := mage.TryCast(t, 133, target, 5*time.Second)
	if err != nil {
		t.Fatalf("offensive cast must produce an explicit failure, got %v", err)
	}
	if result.Success {
		t.Fatal("human Fireball was allowed")
	}
	current, _ := warrior.UnitHP(target)
	if current != hp {
		t.Fatalf("human actions changed target HP: %d -> %d", hp, current)
	}
	if err := warrior.World.SetTarget(target); err != nil {
		t.Fatal(err)
	}
	selectOption(t, warrior, device(t, warrior), "Atacar mi objetivo")
	deadline := time.Now().Add(5 * time.Second)
	for e2eharness.UnitTargetGUID(warrior.World, companion) != target {
		if time.Now().After(deadline) {
			object := warrior.World.GetObject(companion)
			if object == nil {
				t.Fatal("companion disappeared after attack order")
			}
			t.Fatalf("companion did not retain the commanded target: victim=%x flags=%x health=%d position=(%.1f,%.1f,%.1f)",
				e2eharness.UnitTargetGUID(warrior.World, companion), object.Value(client.UnitFieldFlags), object.Value(client.UnitFieldHealth), object.PosX, object.PosY, object.PosZ)
		}
		time.Sleep(30 * time.Millisecond)
	}
	warrior.WaitUnitDead(t, target, 45*time.Second)
	warrior.Save(t)
	var xp uint32
	if err := warrior.CharDB.QueryRow("SELECT xp FROM characters WHERE guid=?", uint32(warrior.World.CharGUID())).Scan(&xp); err != nil {
		t.Fatal(err)
	}
	targetLevel := warrior.World.GetObject(target).Value(client.UnitFieldLevel)
	if targetLevel < 1 || targetLevel > 2 {
		t.Fatalf("unexpected stock fixture level: %d", targetLevel)
	}
	normalXP := ((50*(20+targetLevel-1))/10 + 1) / 2 // Verified core BaseGain for level-one human, 1–60 content.
	if xp != normalXP*70/100 {
		t.Fatalf("human XP must apply 0.70 exactly once: normal=%d received=%d", normalXP, xp)
	}
	var companionLevel, ownerLevel uint8
	var companionXP uint64
	if err := warrior.CharDB.QueryRow("SELECT level,experience FROM digimon_collection WHERE owner_guid=?", uint32(warrior.World.CharGUID())).Scan(&companionLevel, &companionXP); err != nil {
		t.Fatal(err)
	}
	if err := warrior.CharDB.QueryRow("SELECT level FROM characters WHERE guid=?", uint32(warrior.World.CharGUID())).Scan(&ownerLevel); err != nil {
		t.Fatal(err)
	}
	if companionLevel != 2 || ownerLevel != 1 || companionXP != uint64(normalXP)-1 {
		t.Fatalf("independent progress failed: owner level=%d companion level=%d xp=%d normal kill=%d", ownerLevel, companionLevel, companionXP, normalXP)
	}
	var questCredit uint32
	if err := warrior.CharDB.QueryRow("SELECT mobcount1 FROM character_queststatus WHERE guid=? AND quest=7", uint32(warrior.World.CharGUID())).Scan(&questCredit); err != nil {
		t.Fatal(err)
	}
	if questCredit != 1 {
		t.Fatalf("companion kill must credit exactly one mission objective, got %d", questCredit)
	}
	warrior.WaitUnitLootable(t, target, 5*time.Second)
	items := warrior.OpenLoot(t, target, 5*time.Second)
	warrior.LootRelease(t, target)
	t.Logf("PASS: owner's real loot window opened with %d item rows after companion-only kill", len(items))
	t.Logf("PASS: three human swings and Fireball blocked; companion %x killed target %x and owner received %d XP", companion, target, xp)
}
