//go:build e2e

// SPDX-License-Identifier: GPL-2.0-or-later
package digimon_test

import (
	"fmt"
	"math"
	"testing"
	"time"

	"github.com/azerothcore/AzerothGhost/e2e/e2eharness"
)

func assertCollection(t *testing.T, bot *e2eharness.ScenarioBot, expected int) {
	t.Helper()
	var count int
	if err := bot.CharDB.QueryRow("SELECT COUNT(*) FROM digimon_collection WHERE owner_guid=?", uint32(bot.World.CharGUID())).Scan(&count); err != nil {
		t.Fatal(err)
	}
	if count != expected {
		t.Fatalf("collection count=%d, expected %d", count, expected)
	}
}

func selectTarget(t *testing.T, bot *e2eharness.ScenarioBot, target uint64) {
	t.Helper()
	if err := bot.World.SetTarget(target); err != nil {
		t.Fatal(err)
	}
	bot.FlushWorld(t)
}

func spawnWild(t *testing.T, bot *e2eharness.ScenarioBot) (uint64, uint32) {
	t.Helper()
	target, spawn := bot.SpawnPersistent(t, 900103, 10*time.Second)
	if spawn == 0 {
		t.Fatal("persistent wild fixture has no spawn identity")
	}
	_, _, _, mapID := bot.Pos()
	// Remove only this test's lifecycle marker once its owned spawn is removed.
	t.Cleanup(func() {
		if _, err := bot.CharDB.Exec("DELETE FROM creature_respawn WHERE guid=? AND mapId=? AND instanceId=0", spawn, mapID); err != nil {
			t.Errorf("owned wild respawn cleanup: %v", err)
		}
		if _, err := bot.CharDB.Exec("DELETE FROM digimon_wild_life WHERE spawn_guid=?", spawn); err != nil {
			t.Errorf("wild lifecycle cleanup: %v", err)
		}
	})
	return target, spawn
}

func weakenWild(t *testing.T, bot *e2eharness.ScenarioBot, target uint64) {
	t.Helper()
	selectTarget(t, bot, target)
	selectOption(t, bot, device(t, bot), "Atacar mi objetivo")
	deadline := time.Now().Add(50 * time.Second)
	for {
		hp, max := bot.UnitHP(target)
		if max > 0 && hp > 0 && hp*100 <= max*25 {
			break
		}
		if max > 0 && hp == 0 {
			t.Fatal("wild died before capture threshold")
		}
		if time.Now().After(deadline) {
			t.Fatalf("wild did not reach capture threshold: %d/%d", hp, max)
		}
		time.Sleep(25 * time.Millisecond)
	}
	selectOption(t, bot, device(t, bot), "Dejar de atacar y seguirme")
}

// Run this suite with CaptureChance=100. Restore the production default of 35
// afterwards. Probability is controlled by configuration, never by bypassing SQL.
func TestCapture_CollectionOwnershipAndSwitch(t *testing.T) {
	bots := e2eharness.NewScenario(t, e2eharness.ScenarioOpts{Prefix: "DGCap", Bots: []e2eharness.BotSpec{
		{Role: "owner", Race: e2eharness.RaceHuman, Class: e2eharness.ClassWarrior, Level: 1},
		{Role: "other", Race: e2eharness.RaceHuman, Class: e2eharness.ClassMage, Level: 1},
	}})
	e2eharness.TeleportAllPad(t, bots, e2eharness.PackagePad(t))
	owner, other := bots[0], bots[1]
	guide := owner.Spawn(t, 900100, 10*time.Second)
	trainer := owner.Spawn(t, 900101, 10*time.Second)
	wild, spawn := spawnWild(t, owner)
	for _, bot := range bots {
		bot.CombatReady(t)
		selectOption(t, bot, hello(t, bot, guide), "Elegir Lobo digital")
		waitOwned(t, bot, 1)
	}
	menu := selectOption(t, owner, hello(t, owner, trainer), "Aprender Mordisco Digital")
	menu = selectOption(t, owner, menu, "Gestionar técnicas")
	selectOption(t, owner, menu, "Equipar técnica: Mordisco Digital")
	selectTarget(t, owner, wild)
	selectOption(t, owner, device(t, owner), "Capturar el salvaje")
	assertCollection(t, owner, 1) // Full health is not capturable.
	selectTarget(t, owner, waitOwned(t, other, 1)[0])
	selectOption(t, owner, device(t, owner), "Capturar el salvaje")
	assertCollection(t, owner, 1) // Another player's owned actor is not a wild.
	weakenWild(t, owner, wild)
	selectTarget(t, other, wild)
	selectOption(t, other, device(t, other), "Capturar el salvaje")
	assertCollection(t, other, 1)
	selectTarget(t, owner, wild)
	stale := device(t, owner)
	captureIndex := optionIndex(t, stale, "Capturar el salvaje")
	selectOption(t, owner, stale, "Capturar el salvaje")
	assertCollection(t, owner, 2)
	sendIndex(t, owner, stale, captureIndex)
	assertCollection(t, owner, 2)
	var captured uint32
	var level uint8
	var health float32
	var claimed uint8
	var ready uint64
	if err := owner.CharDB.QueryRow("SELECT id,level,health_pct FROM digimon_collection WHERE owner_guid=? AND is_starter=0", uint32(owner.World.CharGUID())).Scan(&captured, &level, &health); err != nil {
		t.Fatal(err)
	}
	if level < 1 || level > 2 || health <= 0 || health > 25 {
		t.Fatalf("captured stats were not preserved: level=%d health=%.2f", level, health)
	}
	if err := owner.CharDB.QueryRow("SELECT claimed FROM digimon_wild_life WHERE spawn_guid=?", spawn).Scan(&claimed); err != nil {
		t.Fatal(err)
	}
	if claimed != 1 {
		t.Fatal("captured life has no durable claim")
	}
	if err := owner.CharDB.QueryRow("SELECT capture_ready_at FROM digimon_tamer WHERE owner_guid=?", uint32(owner.World.CharGUID())).Scan(&ready); err != nil {
		t.Fatal(err)
	}
	if ready == 0 {
		t.Fatal("capture cooldown was not persisted")
	}
	owner.Relog(t)
	assertCollection(t, owner, 2)
	var restored uint64
	if err := owner.CharDB.QueryRow("SELECT capture_ready_at FROM digimon_tamer WHERE owner_guid=?", uint32(owner.World.CharGUID())).Scan(&restored); err != nil {
		t.Fatal(err)
	}
	if ready != restored {
		t.Fatal("relogin reset capture cooldown")
	}
	oldActor := waitOwned(t, owner, 1)[0]
	menu = selectOption(t, owner, device(t, owner), "Consultar colección")
	selectOption(t, owner, menu, fmt.Sprintf("Activar Lobo digital #%d", captured))
	actors := waitOwned(t, owner, 1)
	if actors[0] == oldActor {
		t.Fatal("switch did not replace the active actor")
	}
	var active uint32
	if err := owner.CharDB.QueryRow("SELECT companion_id FROM digimon_active WHERE owner_guid=?", uint32(owner.World.CharGUID())).Scan(&active); err != nil {
		t.Fatal(err)
	}
	if active != captured {
		t.Fatalf("active companion=%d, expected %d", active, captured)
	}
	// The initial CREATE_OBJECT packet precedes ApplyStats. Wait for the
	// subsequent value update before judging the captured actor's health.
	deadline := time.Now().Add(5 * time.Second)
	hp, max := owner.UnitHP(actors[0])
	expectedHealth := uint32(math.Round(float64(60+25*uint32(level)) * float64(health) / 100))
	for (max != 60+25*uint32(level) || hp != expectedHealth) && time.Now().Before(deadline) {
		time.Sleep(25 * time.Millisecond)
		hp, max = owner.UnitHP(actors[0])
	}
	if max != 60+25*uint32(level) || hp != expectedHealth {
		t.Fatalf("switch changed captured companion health: %d/%d, expected %d", hp, max, expectedHealth)
	}
	owner.Relog(t)
	waitOwned(t, owner, 1)
	assertCollection(t, owner, 2)
	assertTraining(t, owner, 80, 1, 1)
	saveRestartFixture(t, bots)
	t.Log("PASS: full-health and owned targets rejected, other Tamer cannot steal capture, unique durable claim, preserved level/health/cooldown, collection switch and relog retain one actor")
}

func TestCapture_FailedWrite(t *testing.T) {
	bot := e2eharness.NewSolo(t, e2eharness.ScenarioOpts{Prefix: "DigiCF", Level: 1})
	bot.TeleportPad(t, e2eharness.PackagePad(t))
	guide := bot.Spawn(t, 900100, 10*time.Second)
	wild, spawn := spawnWild(t, bot)
	bot.CombatReady(t)
	selectOption(t, bot, hello(t, bot, guide), "Elegir Lobo digital")
	owner := uint32(bot.World.CharGUID())
	var ready uint64
	weakenWild(t, bot, wild)
	name := fmt.Sprintf("digimon_e2e_capture_%d", owner)
	statement := fmt.Sprintf("CREATE TRIGGER `%s` BEFORE INSERT ON digimon_collection FOR EACH ROW BEGIN IF NEW.owner_guid=%d AND NEW.is_starter=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='expected capture rollback test'; END IF; END", name, owner)
	if _, err := bot.CharDB.Exec(statement); err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		if _, err := bot.CharDB.Exec("DROP TRIGGER `" + name + "`"); err != nil {
			t.Errorf("capture trigger cleanup: %v", err)
		}
	})
	selectTarget(t, bot, wild)
	selectOption(t, bot, device(t, bot), "Capturar el salvaje")
	assertCollection(t, bot, 1)
	var claimed uint8
	if err := bot.CharDB.QueryRow("SELECT claimed FROM digimon_wild_life WHERE spawn_guid=?", spawn).Scan(&claimed); err != nil {
		t.Fatal(err)
	}
	if claimed != 0 {
		t.Fatal("failed collection write claimed the wild")
	}
	if err := bot.CharDB.QueryRow("SELECT capture_ready_at FROM digimon_tamer WHERE owner_guid=?", owner).Scan(&ready); err != nil {
		t.Fatal(err)
	}
	if ready != 0 {
		t.Fatal("failed collection write consumed cooldown")
	}
	hp, _ := bot.UnitHP(wild)
	if hp == 0 {
		t.Fatal("failed capture removed or killed the wild")
	}
	t.Log("PASS: injected SQL failure rolls back cooldown, collection and wild claim, leaving target alive")
}

func TestCapture_FullCollection(t *testing.T) {
	bot := e2eharness.NewSolo(t, e2eharness.ScenarioOpts{Prefix: "DGFl", Level: 1})
	bot.TeleportPad(t, e2eharness.PackagePad(t))
	guide := bot.Spawn(t, 900100, 10*time.Second)
	wild, _ := spawnWild(t, bot)
	bot.CombatReady(t)
	selectOption(t, bot, hello(t, bot, guide), "Elegir Lobo digital")
	owner := uint32(bot.World.CharGUID())
	if _, err := bot.CharDB.Exec("UPDATE digimon_tamer SET collection_capacity=1 WHERE owner_guid=?", owner); err != nil {
		t.Fatal(err)
	}
	bot.Relog(t)
	bot.CombatReady(t)
	weakenWild(t, bot, wild)
	selectTarget(t, bot, wild)
	selectOption(t, bot, device(t, bot), "Capturar el salvaje")
	assertCollection(t, bot, 1)
	var ready uint64
	if err := bot.CharDB.QueryRow("SELECT capture_ready_at FROM digimon_tamer WHERE owner_guid=?", owner).Scan(&ready); err != nil {
		t.Fatal(err)
	}
	if ready != 0 {
		t.Fatal("full collection consumed a capture attempt")
	}
	t.Log("PASS: full collection rejects a weakened authorized wild without consuming cooldown")
}
