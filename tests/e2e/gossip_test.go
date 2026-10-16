//go:build e2e

// SPDX-License-Identifier: GPL-2.0-or-later
package digimon_test

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"strings"
	"testing"
	"time"

	"github.com/azerothcore/AzerothGhost/client"
	"github.com/azerothcore/AzerothGhost/e2e/e2eharness"
)

// Values and layouts are checked against the pinned AzerothCore Opcodes.h and GossipDef.cpp.
const gossipHello uint16 = 0x17b
const gossipSelect uint16 = 0x17c
const gossipMessage uint16 = 0x17d

type gossipOption struct {
	Index uint32
	Label string
}
type gossipMenu struct {
	Source  uint64
	ID      uint32
	Options []gossipOption
}

func readCString(reader *bytes.Reader) (string, error) {
	var out []byte
	for reader.Len() > 0 {
		next, err := reader.ReadByte()
		if err != nil {
			return "", err
		}
		if next == 0 {
			return string(out), nil
		}
		out = append(out, next)
	}
	return "", fmt.Errorf("unterminated gossip string")
}

func parseMenu(data []byte) (gossipMenu, error) {
	var menu gossipMenu
	reader := bytes.NewReader(data)
	var title, count uint32
	for _, field := range []any{&menu.Source, &menu.ID, &title, &count} {
		if err := binary.Read(reader, binary.LittleEndian, field); err != nil {
			return menu, err
		}
	}
	if count > 64 {
		return menu, fmt.Errorf("unexpected option count %d", count)
	}
	for i := uint32(0); i < count; i++ {
		var option gossipOption
		var icon, coded uint8
		var money uint32
		for _, field := range []any{&option.Index, &icon, &coded, &money} {
			if err := binary.Read(reader, binary.LittleEndian, field); err != nil {
				return menu, err
			}
		}
		var err error
		option.Label, err = readCString(reader)
		if err != nil {
			return menu, err
		}
		if _, err = readCString(reader); err != nil {
			return menu, err
		}
		menu.Options = append(menu.Options, option)
	}
	return menu, nil
}

func receiveMenu(t *testing.T, bot *e2eharness.ScenarioBot, send func() error) gossipMenu {
	t.Helper()
	received := make(chan gossipMenu, 1)
	parseErrors := make(chan error, 1)
	cancel := bot.World.AddPacketHook(func(op uint16, data []byte) {
		if op != gossipMessage {
			return
		}
		menu, err := parseMenu(data)
		if err != nil {
			select {
			case parseErrors <- err:
			default:
			}
			return
		}
		select {
		case received <- menu:
		default:
		}
	})
	defer cancel()
	if err := send(); err != nil {
		t.Fatalf("send gossip: %v", err)
	}
	select {
	case menu := <-received:
		return menu
	case err := <-parseErrors:
		t.Fatalf("parse actual gossip packet: %v", err)
	case <-time.After(10 * time.Second):
		t.Fatal("server did not return a gossip menu")
	}
	return gossipMenu{}
}

func hello(t *testing.T, bot *e2eharness.ScenarioBot, guid uint64) gossipMenu {
	return receiveMenu(t, bot, func() error {
		data := make([]byte, 8)
		binary.LittleEndian.PutUint64(data, guid)
		return bot.World.SendPacketRaw(gossipHello, data)
	})
}

func device(t *testing.T, bot *e2eharness.ScenarioBot) gossipMenu {
	return receiveMenu(t, bot, func() error { return bot.World.SendGMCommand(".digimon") })
}

func selectOption(t *testing.T, bot *e2eharness.ScenarioBot, menu gossipMenu, label string) gossipMenu {
	t.Helper()
	var selected *gossipOption
	for i := range menu.Options {
		if strings.Contains(menu.Options[i].Label, label) {
			selected = &menu.Options[i]
			break
		}
	}
	if selected == nil {
		t.Fatalf("option %q missing in %+v", label, menu.Options)
	}
	return receiveMenu(t, bot, func() error {
		data := make([]byte, 16)
		binary.LittleEndian.PutUint64(data, menu.Source)
		binary.LittleEndian.PutUint32(data[8:], menu.ID)
		binary.LittleEndian.PutUint32(data[12:], selected.Index)
		return bot.World.SendPacketRaw(gossipSelect, data)
	})
}

func ownedCompanions(bot *e2eharness.ScenarioBot) []uint64 {
	return ownedByEntry(bot, 900102)
}

func ownedByEntry(bot *e2eharness.ScenarioBot, entry uint32) []uint64 {
	var out []uint64
	for _, unit := range bot.UnitsByEntry(150, entry) {
		object := bot.World.GetObject(unit.GUID)
		if object == nil {
			continue
		}
		owner := uint64(object.Value(client.UnitFieldSummonedBy)) |
			uint64(object.Value(client.UnitFieldSummonedBy+1))<<32
		if owner == bot.World.CharGUID() {
			out = append(out, unit.GUID)
		}
	}
	return out
}

func waitOwned(t *testing.T, bot *e2eharness.ScenarioBot, expected int) []uint64 {
	t.Helper()
	deadline := time.NewTimer(10 * time.Second)
	defer deadline.Stop()
	tick := time.NewTicker(30 * time.Millisecond)
	defer tick.Stop()
	for {
		units := ownedCompanions(bot)
		if len(units) == expected {
			return units
		}
		select {
		case <-tick.C:
		case <-deadline.C:
			t.Fatalf("expected %d owned companions, got %v", expected, units)
		}
	}
}
