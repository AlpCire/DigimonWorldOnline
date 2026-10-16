//go:build e2e

// SPDX-License-Identifier: GPL-2.0-or-later
package digimon_test

import (
	"testing"
	"time"

	"github.com/azerothcore/AzerothGhost/e2e/e2eharness"
	_ "github.com/go-sql-driver/mysql"
)

func TestTamer_TwoClassesUniqueStarterAndRelog(t *testing.T) {
	bots := e2eharness.NewScenario(t, e2eharness.ScenarioOpts{Prefix: "DigiA", Bots: []e2eharness.BotSpec{
		{Role: "warrior", Race: e2eharness.RaceHuman, Class: e2eharness.ClassWarrior, Level: 1},
		{Role: "mage", Race: e2eharness.RaceHuman, Class: e2eharness.ClassMage, Level: 1},
	}})
	e2eharness.TeleportAllPad(t, bots, e2eharness.PackagePad(t))
	// Both connected clients share the same welcoming NPC. Creating a second
	// identical spawn at this position can reuse a GUID cached by the harness.
	guide := bots[0].Spawn(t, 900100, 10*time.Second)
	for _, bot := range bots {
		menu := hello(t, bot, guide)
		menu = selectOption(t, bot, menu, "Elegir Lobo digital")
		actor := waitOwned(t, bot, 1)[0]
		var count int
		if err := bot.CharDB.QueryRow("SELECT COUNT(*) FROM digimon_collection WHERE owner_guid=?", uint32(bot.World.CharGUID())).Scan(&count); err != nil {
			t.Fatal(err)
		}
		if count != 1 {
			t.Fatalf("starter selection persisted %d companions", count)
		}
		for i := 0; i < 3; i++ {
			menu = selectOption(t, bot, menu, "Invocar compañero")
		}
		units := waitOwned(t, bot, 1)
		if units[0] != actor {
			t.Fatal("repeated summon replaced a living owned actor")
		}
		if err := bot.CharDB.QueryRow("SELECT COUNT(*) FROM digimon_collection WHERE owner_guid=?", uint32(bot.World.CharGUID())).Scan(&count); err != nil {
			t.Fatal(err)
		}
		if count != 1 {
			t.Fatal("repeated summon duplicated persistent collection")
		}
		bot.Save(t)
		bot.Relog(t)
		waitOwned(t, bot, 1)
		menu = device(t, bot)
		menu = selectOption(t, bot, menu, "Retirar compañero")
		waitOwned(t, bot, 0)
		bot.Relog(t)
		waitOwned(t, bot, 0)
		menu = device(t, bot)
		selectOption(t, bot, menu, "Invocar compañero")
		waitOwned(t, bot, 1)
		t.Logf("PASS: %s selected once; repeated summon, relog, withdraw and relog preserve state", bot.Role)
	}
	if !bots[0].Alive() || !bots[1].Alive() {
		t.Fatal("two sessions did not remain connected")
	}
	t.Log("PASS: warrior and mage sessions are simultaneously connected with independent companions")
	saveRestartFixture(t, bots)
}
