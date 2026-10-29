//go:build e2e

// SPDX-License-Identifier: GPL-2.0-or-later
package digimon_test

import (
	"testing"
	"time"

	"github.com/azerothcore/AzerothGhost/client"
	"github.com/azerothcore/AzerothGhost/e2e/e2eharness"
)

// Run on the disposable laboratory with a temporary enabled rule 1 -> 2 at 22,
// species 2 enabled as Champion. Restore configuration after this suite.
func requireEvolutionFixture(t *testing.T, bot *e2eharness.ScenarioBot) {
	t.Helper()
	var count int
	if err := bot.CharDB.QueryRow(`SELECT COUNT(*) FROM digimon_evolution_rules r
        JOIN digimon_species s ON s.id=r.target_species WHERE r.source_species=1
        AND r.target_species=2 AND r.required_level=22 AND r.enabled=1 AND s.enabled=1 AND s.stage=2`).Scan(&count); err != nil {
		t.Fatal(err)
	}
	if count != 1 {
		t.Skip("requires isolated evolution fixture; do not enable on the player catalog")
	}
}

func adminCommand(t *testing.T, bot *e2eharness.ScenarioBot, command string) {
	t.Helper()
	bot.GM(t, command)
	bot.FlushWorld(t)
}

func assertForm(t *testing.T, bot *e2eharness.ScenarioBot, species, level uint32, minimumXP uint64) {
	t.Helper()
	var gotSpecies, gotLevel, stage uint32
	var xp uint64
	if err := bot.CharDB.QueryRow("SELECT species_id,level,stage,experience FROM digimon_collection WHERE owner_guid=?", uint32(bot.World.CharGUID())).Scan(&gotSpecies, &gotLevel, &stage, &xp); err != nil {
		t.Fatal(err)
	}
	if gotSpecies != species || gotLevel != level || xp < minimumXP || (species == 2 && stage != 2) {
		t.Fatalf("form=%d stage=%d level=%d xp=%d expected species=%d level=%d xp>=%d", gotSpecies, stage, gotLevel, xp, species, level, minimumXP)
	}
	var expectedEntry uint32
	if err := bot.CharDB.QueryRow("SELECT creature_entry FROM digimon_species WHERE id=?", species).Scan(&expectedEntry); err != nil {
		t.Fatal(err)
	}
	deadline := time.Now().Add(5 * time.Second)
	for {
		actors := ownedByEntry(bot, expectedEntry)
		if len(actors) == 1 {
			object := bot.World.GetObject(actors[0])
			if object != nil && object.Value(client.UnitFieldLevel) == level && object.Value(client.UnitFieldEntry) == expectedEntry {
				break
			}
		}
		if time.Now().After(deadline) {
			t.Fatal("live actor did not receive the persisted form and level")
		}
		time.Sleep(25 * time.Millisecond)
	}
}

func TestTamer_AdminPreviewAndLevelEvolution(t *testing.T) {
	bot := e2eharness.NewSolo(t, e2eharness.ScenarioOpts{Prefix: "DGEAdm", Level: 1})
	requireEvolutionFixture(t, bot)
	bot.TeleportPad(t, e2eharness.PackagePad(t))
	guide := bot.Spawn(t, 900100, 10*time.Second)
	bot.CombatReady(t)
	selectOption(t, bot, hello(t, bot, guide), "Elegir Lobo digital")
	principal := waitOwned(t, bot, 1)[0]
	for i := 0; i < 7; i++ {
		adminCommand(t, bot, ".spawndigi Lobo digital 12")
	}
	if units := bot.UnitsByEntry(100, 900121); len(units) != 5 {
		t.Fatalf("preview cap: %d", len(units))
	}
	readyDeadline := time.Now().Add(5 * time.Second)
	for {
		ready := 0
		for _, unit := range bot.UnitsByEntry(100, 900121) {
			if object := bot.World.GetObject(unit.GUID); object != nil && object.Value(client.UnitFieldLevel) == 12 {
				ready++
			}
		}
		if ready == 5 {
			break
		}
		if time.Now().After(readyDeadline) {
			t.Fatal("preview level missing")
		}
		time.Sleep(25 * time.Millisecond)
	}
	assertCollection(t, bot, 1)
	if waitOwned(t, bot, 1)[0] != principal {
		t.Fatal("preview replaced operational companion")
	}
	adminCommand(t, bot, ".spawndigi borrar")
	deadline := time.Now().Add(5 * time.Second)
	for len(bot.UnitsByEntry(100, 900121)) != 0 {
		if time.Now().After(deadline) {
			t.Fatal("preview cleanup failed")
		}
		time.Sleep(25 * time.Millisecond)
	}
	adminCommand(t, bot, ".spawndigi Patamon")
	if len(bot.UnitsByEntry(100, 900121)) != 0 {
		t.Fatal("missing Digimon silently used a placeholder model")
	}
	adminCommand(t, bot, ".digilevel 21")
	assertForm(t, bot, 1, 21, 0)
	adminCommand(t, bot, ".digilevel 22")
	assertForm(t, bot, 2, 22, 0)
	bot.Relog(t)
	assertForm(t, bot, 2, 22, 0)
	adminCommand(t, bot, ".digilevel 0")
	assertForm(t, bot, 2, 22, 0)
	var account uint32
	if err := bot.AuthDB.QueryRow("SELECT id FROM account WHERE username=?", bot.Ident.Account).Scan(&account); err != nil {
		t.Fatal(err)
	}
	if _, err := bot.AuthDB.Exec("DELETE FROM account_access WHERE id=?", account); err != nil {
		t.Fatal(err)
	}
	bot.Relog(t)
	if err := bot.World.SendGMCommand(".digilevel 30"); err != nil {
		t.Fatal(err)
	}
	if err := bot.World.SendGMCommand(".spawndigi 1"); err != nil {
		t.Fatal(err)
	}
	// Normal accounts cannot use .gps for FlushWorld. Relog supplies a real
	// persistence boundary after both denied commands have been sent.
	bot.Relog(t)
	assertForm(t, bot, 2, 22, 0)
	if len(bot.UnitsByEntry(100, 900121)) != 0 {
		t.Fatal("normal player could create admin previews")
	}
	t.Log("PASS: named previews capped and removable, no collection mutation, missing model rejected; admin 21->22 evolution persists; invalid level and normal-account commands denied")
}

func TestTamer_NaturalLevelEvolutionKeepsOverflowXP(t *testing.T) {
	bot := e2eharness.NewSolo(t, e2eharness.ScenarioOpts{Prefix: "DGENat", Level: 21})
	requireEvolutionFixture(t, bot)
	bot.TeleportPad(t, e2eharness.PackagePad(t))
	guide := bot.Spawn(t, 900100, 10*time.Second)
	target := bot.Spawn(t, 6, 10*time.Second)
	selectTarget(t, bot, target)
	adminCommand(t, bot, ".npc set level 22")
	bot.CombatReady(t)
	selectOption(t, bot, hello(t, bot, guide), "Elegir Lobo digital")
	adminCommand(t, bot, ".digilevel 21")
	if _, err := bot.CharDB.Exec("UPDATE digimon_collection SET experience=1674 WHERE owner_guid=?", uint32(bot.World.CharGUID())); err != nil {
		t.Fatal(err)
	}
	bot.Relog(t)
	bot.CombatReady(t)
	assertForm(t, bot, 1, 21, 1674)
	selectTarget(t, bot, target)
	selectOption(t, bot, device(t, bot), "Atacar mi objetivo")
	bot.WaitUnitDead(t, target, 50*time.Second)
	// KilledUnit persists the natural XP and species in one transaction.
	deadline := time.Now().Add(5 * time.Second)
	for {
		var species uint32
		if err := bot.CharDB.QueryRow("SELECT species_id FROM digimon_collection WHERE owner_guid=?", uint32(bot.World.CharGUID())).Scan(&species); err != nil {
			t.Fatal(err)
		}
		if species == 2 {
			break
		}
		if time.Now().After(deadline) {
			t.Fatal("natural level-up did not evolve")
		}
		time.Sleep(25 * time.Millisecond)
	}
	assertForm(t, bot, 2, 22, 1)
	bot.CombatStop(t)
	bot.Relog(t)
	assertForm(t, bot, 2, 22, 1)
	t.Log("PASS: actual companion kill crosses 21->22, changes form, preserves overflow XP and survives relog; native test models only")
}
