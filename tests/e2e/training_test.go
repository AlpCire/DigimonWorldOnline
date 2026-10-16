//go:build e2e

// SPDX-License-Identifier: GPL-2.0-or-later
package digimon_test

import (
	"encoding/binary"
	"fmt"
	"strings"
	"testing"
	"time"

	"github.com/azerothcore/AzerothGhost/e2e/e2eharness"
)

func optionIndex(t *testing.T, menu gossipMenu, label string) uint32 {
	t.Helper()
	for _, option := range menu.Options {
		if strings.Contains(option.Label, label) {
			return option.Index
		}
	}
	t.Fatalf("missing option %q", label)
	return 0
}

func sendIndex(t *testing.T, bot *e2eharness.ScenarioBot, menu gossipMenu, index uint32) {
	t.Helper()
	data := make([]byte, 16)
	binary.LittleEndian.PutUint64(data, menu.Source)
	binary.LittleEndian.PutUint32(data[8:], menu.ID)
	binary.LittleEndian.PutUint32(data[12:], index)
	if err := bot.World.SendPacketRaw(gossipSelect, data); err != nil {
		t.Fatal(err)
	}
	bot.FlushWorld(t)
}

func assertTraining(t *testing.T, bot *e2eharness.ScenarioBot, bits uint64, learned, equipped int) {
	t.Helper()
	owner := uint32(bot.World.CharGUID())
	var wallet uint64
	var known, used int
	if err := bot.CharDB.QueryRow("SELECT bits FROM digimon_tamer WHERE owner_guid=?", owner).Scan(&wallet); err != nil {
		t.Fatal(err)
	}
	if err := bot.CharDB.QueryRow("SELECT COUNT(*) FROM digimon_learned l JOIN digimon_collection c ON c.id=l.companion_id WHERE c.owner_guid=?", owner).Scan(&known); err != nil {
		t.Fatal(err)
	}
	if err := bot.CharDB.QueryRow("SELECT COUNT(*) FROM digimon_equipped e JOIN digimon_collection c ON c.id=e.companion_id WHERE c.owner_guid=?", owner).Scan(&used); err != nil {
		t.Fatal(err)
	}
	if wallet != bits || known != learned || used != equipped {
		t.Fatalf("wallet/learned/equipped = %d/%d/%d, expected %d/%d/%d", wallet, known, used, bits, learned, equipped)
	}
}

func TestTamer_TrainerTransactionsEquipmentAndSpell(t *testing.T) {
	bot := e2eharness.NewSolo(t, e2eharness.ScenarioOpts{Prefix: "DigiT", Level: 1})
	bot.TeleportPad(t, e2eharness.PackagePad(t))
	guide := bot.Spawn(t, 900100, 10*time.Second)
	trainer := bot.Spawn(t, 900101, 10*time.Second)
	bot.CombatReady(t)
	selectOption(t, bot, hello(t, bot, guide), "Elegir Lobo digital")
	actor := waitOwned(t, bot, 1)[0]
	assertTraining(t, bot, 100, 0, 0)
	for _, option := range device(t, bot).Options {
		if strings.Contains(option.Label, "Aprender") {
			t.Fatal("device offers trainer-only learning")
		}
	}
	menu := hello(t, bot, trainer)
	for _, option := range menu.Options {
		if strings.Contains(option.Label, "Garra Feroz") || strings.Contains(option.Label, "Colmillo Rompedatos") || strings.Contains(option.Label, "Furia del Núcleo") {
			t.Fatal("higher stage is offered to Rookie")
		}
	}
	stale := menu
	index := optionIndex(t, menu, "Aprender Mordisco Digital")
	menu = selectOption(t, bot, menu, "Aprender Mordisco Digital")
	assertTraining(t, bot, 80, 1, 0)
	sendIndex(t, bot, stale, index) // Replay the identical wire-level purchase packet.
	assertTraining(t, bot, 80, 1, 0)
	menu = selectOption(t, bot, menu, "Aprender Aullido de Guerra")
	assertTraining(t, bot, 45, 2, 0)
	menu = selectOption(t, bot, menu, "Gestionar técnicas")
	menu = selectOption(t, bot, menu, "Equipar técnica: Mordisco Digital")
	menu = selectOption(t, bot, menu, "Gestionar técnicas")
	menu = selectOption(t, bot, menu, "Equipar técnica: Aullido de Guerra")
	assertTraining(t, bot, 45, 2, 2)
	selectOption(t, bot, menu, "Usar: Aullido de Guerra")
	e2eharness.WaitUnitAura(t, bot.World, actor, 24604, 5*time.Second)
	bot.Relog(t)
	waitOwned(t, bot, 1)
	assertTraining(t, bot, 45, 2, 2)
	var highestSlot int
	if err := bot.CharDB.QueryRow("SELECT MAX(e.slot) FROM digimon_equipped e JOIN digimon_collection c ON c.id=e.companion_id WHERE c.owner_guid=?", uint32(bot.World.CharGUID())).Scan(&highestSlot); err != nil {
		t.Fatal(err)
	}
	if highestSlot != 1 {
		t.Fatalf("Rookie must use exactly slots zero and one, got highest %d", highestSlot)
	}
	t.Log("PASS: trainer-only purchases, stale-packet replay, separate learned/equipped states, two Rookie slots, actual Furious Howl aura and relog persistence")
}

func TestTamer_InsufficientFundsAndForgedHigherTechnique(t *testing.T) {
	bot := e2eharness.NewSolo(t, e2eharness.ScenarioOpts{Prefix: "DigiP", Level: 1})
	pad := e2eharness.PackagePad(t)
	bot.TeleportPad(t, pad)
	guide := bot.Spawn(t, 900100, 10*time.Second)
	trainer := bot.Spawn(t, 900101, 10*time.Second)
	bot.CombatReady(t)
	selectOption(t, bot, hello(t, bot, guide), "Elegir Lobo digital")
	if _, err := bot.CharDB.Exec("UPDATE digimon_tamer SET bits=0 WHERE owner_guid=?", uint32(bot.World.CharGUID())); err != nil {
		t.Fatal(err)
	}
	bot.Relog(t)
	menu := hello(t, bot, trainer)
	menu = selectOption(t, bot, menu, "Aprender Mordisco Digital")
	assertTraining(t, bot, 0, 0, 0)
	sendIndex(t, bot, menu, (3<<24)|3) // Disabled Champion technique, absent from the server menu.
	assertTraining(t, bot, 0, 0, 0)
	if _, err := bot.CharDB.Exec("UPDATE digimon_tamer SET bits=100 WHERE owner_guid=?", uint32(bot.World.CharGUID())); err != nil {
		t.Fatal(err)
	}
	bot.Relog(t)
	menu = hello(t, bot, trainer)
	sendIndex(t, bot, menu, (3<<24)|3)
	assertTraining(t, bot, 100, 0, 0)
	index := optionIndex(t, menu, "Aprender Mordisco Digital")
	bot.Teleport(t, pad.X+200, pad.Y, pad.Z, pad.Map)
	sendIndex(t, bot, menu, index)
	assertTraining(t, bot, 100, 0, 0)
	t.Log("PASS: insufficient funds, forged higher-stage purchase and remote trainer replay cannot grant or debit a technique")
}

func TestTamer_PurchaseRollsBackWhenLearningWriteFails(t *testing.T) {
	bot := e2eharness.NewSolo(t, e2eharness.ScenarioOpts{Prefix: "DigiR", Level: 1})
	bot.TeleportPad(t, e2eharness.PackagePad(t))
	guide := bot.Spawn(t, 900100, 10*time.Second)
	trainer := bot.Spawn(t, 900101, 10*time.Second)
	bot.CombatReady(t)
	selectOption(t, bot, hello(t, bot, guide), "Elegir Lobo digital")
	var companion uint32
	owner := uint32(bot.World.CharGUID())
	if err := bot.CharDB.QueryRow("SELECT id FROM digimon_collection WHERE owner_guid=?", owner).Scan(&companion); err != nil {
		t.Fatal(err)
	}
	// Fault injection is confined to this disposable companion. The trigger is
	// always removed and never rejects another account's purchases.
	name := fmt.Sprintf("digimon_e2e_reject_%d", owner)
	sql := fmt.Sprintf("CREATE TRIGGER `%s` BEFORE INSERT ON digimon_learned FOR EACH ROW BEGIN IF NEW.companion_id = %d THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'expected Digimon rollback test'; END IF; END", name, companion)
	if _, err := bot.CharDB.Exec(sql); err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		if _, err := bot.CharDB.Exec("DROP TRIGGER `" + name + "`"); err != nil {
			t.Errorf("remove owned fault-injection trigger: %v", err)
		}
	})
	selectOption(t, bot, hello(t, bot, trainer), "Aprender Mordisco Digital")
	assertTraining(t, bot, 100, 0, 0)
	var serial uint64
	if err := bot.CharDB.QueryRow("SELECT purchase_serial FROM digimon_tamer WHERE owner_guid=?", owner).Scan(&serial); err != nil {
		t.Fatal(err)
	}
	if serial != 0 {
		t.Fatalf("debit marker survived rolled-back transaction: %d", serial)
	}
	t.Log("PASS: injected learning-write failure rolls back the wallet debit and transaction marker")
}
