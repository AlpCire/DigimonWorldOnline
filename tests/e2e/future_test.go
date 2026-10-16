//go:build e2e

// SPDX-License-Identifier: GPL-2.0-or-later
package digimon_test

import (
	"testing"
	"time"

	"github.com/azerothcore/AzerothGhost/e2e/e2eharness"
)

// Run separately with ExperimentsEnable=1, then restore zero. These three
// temporary followers cannot fight or become persistent collection members.
func TestLab_ThreeHarmlessFollowers(t *testing.T) {
	bot := e2eharness.NewSolo(t, e2eharness.ScenarioOpts{Prefix: "DGLab", Level: 1})
	pad := e2eharness.PackagePad(t)
	bot.TeleportPad(t, pad)
	guide := bot.Spawn(t, 900100, 10*time.Second)
	bot.CombatReady(t)
	selectOption(t, bot, hello(t, bot, guide), "Elegir Lobo digital")
	principal := waitOwned(t, bot, 1)[0]
	before := persistentSnapshot(t, bot.CharDB, uint32(bot.World.CharGUID()))
	bot.GM(t, ".digimonlab")
	deadline := time.Now().Add(5 * time.Second)
	var actors []uint64
	for {
		actors = ownedByEntry(bot, 900120)
		if len(actors) == 3 {
			break
		}
		if time.Now().After(deadline) {
			t.Fatalf("expected 3 experiment followers, got %v", actors)
		}
		time.Sleep(25 * time.Millisecond)
	}
	bot.GM(t, ".digimonlab")
	bot.FlushWorld(t)
	if count := len(ownedByEntry(bot, 900120)); count != 3 {
		t.Fatalf("repeated experiment duplicated followers: %d", count)
	}
	// A same-map relocation leaves the followers to move through their own AI.
	// Observe their real spline positions reaching the owner's new location.
	bot.Teleport(t, pad.X+15, pad.Y, pad.Z, pad.Map)
	deadline = time.Now().Add(10 * time.Second)
	for {
		arrived := 0
		x, y, z, _ := bot.Pos()
		for _, guid := range actors {
			object := bot.World.GetObject(guid)
			if object == nil {
				continue
			}
			ax, ay, az := object.InterpolatedPosition()
			if e2eharness.Distance3D(x, y, z, ax, ay, az) < 7 {
				arrived++
			}
		}
		if arrived == 3 {
			break
		}
		if time.Now().After(deadline) {
			t.Fatalf("only %d/3 followers reached relocated owner", arrived)
		}
		time.Sleep(50 * time.Millisecond)
	}
	if active := waitOwned(t, bot, 1)[0]; active != principal {
		t.Fatal("experiment replaced the operational companion")
	}
	deadline = time.Now().Add(35 * time.Second)
	for len(ownedByEntry(bot, 900120)) != 0 {
		if time.Now().After(deadline) {
			t.Fatal("temporary followers did not expire")
		}
		time.Sleep(100 * time.Millisecond)
	}
	after := persistentSnapshot(t, bot.CharDB, uint32(bot.World.CharGUID()))
	if before != after {
		t.Fatalf("experiment changed persistent progression: before=%s after=%s", before, after)
	}
	t.Log("PASS: 3 distinct owned followers move, repeated command is bounded, all expire; one operational companion and all persistent data remain unchanged")
}

func TestTamer_NormalPlayerDeviceAndLabDenied(t *testing.T) {
	bot := e2eharness.NewSolo(t, e2eharness.ScenarioOpts{Prefix: "DGN", Level: 1})
	bot.TeleportPad(t, e2eharness.PackagePad(t))
	guide := bot.Spawn(t, 900100, 10*time.Second)
	bot.CombatReady(t)
	selectOption(t, bot, hello(t, bot, guide), "Elegir Lobo digital")
	var account uint32
	if err := bot.AuthDB.QueryRow("SELECT id FROM account WHERE username=?", bot.Ident.Account).Scan(&account); err != nil {
		t.Fatal(err)
	}
	if _, err := bot.AuthDB.Exec("DELETE FROM account_access WHERE id=?", account); err != nil {
		t.Fatal(err)
	}
	bot.Relog(t)
	waitOwned(t, bot, 1)
	menu := device(t, bot)
	menu = selectOption(t, bot, menu, "Retirar compañero")
	waitOwned(t, bot, 0)
	selectOption(t, bot, menu, "Invocar compañero")
	waitOwned(t, bot, 1)
	if err := bot.World.SendGMCommand(".digimonlab"); err != nil {
		t.Fatal(err)
	}
	bot.FlushWorld(t)
	if len(ownedByEntry(bot, 900120)) != 0 {
		t.Fatal("normal account could start administrator experiment")
	}
	t.Log("PASS: account with no GM access can use device, withdraw and summon; cannot start lab experiment")
}
