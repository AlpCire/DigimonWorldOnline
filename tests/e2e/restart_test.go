//go:build e2e

// SPDX-License-Identifier: GPL-2.0-or-later
package digimon_test

import (
	"database/sql"
	"encoding/json"
	"math"
	"os"
	"testing"
	"time"

	"github.com/azerothcore/AzerothGhost/e2e/e2eharness"
)

type restartFixture struct {
	Identity e2eharness.BotIdent
	Owner    uint32
	Snapshot string
}

func persistentSnapshot(t *testing.T, db *sql.DB, owner uint32) string {
	t.Helper()
	queries := []string{
		`SELECT JSON_OBJECT('active',a.companion_id,'bits',t.bits,'summoned',t.summoned,
            'capacity',t.collection_capacity,'captureReady',t.capture_ready_at) FROM digimon_tamer t
            JOIN digimon_active a ON a.owner_guid=t.owner_guid WHERE t.owner_guid=?`,
		`SELECT JSON_OBJECT('id',id,'species',species_id,'stage',stage,'level',level,
            'xp',experience,'health',health_pct,'capture',capture_key) FROM digimon_collection WHERE owner_guid=? ORDER BY id`,
		`SELECT JSON_OBJECT('companion',l.companion_id,'technique',l.technique_id)
            FROM digimon_learned l JOIN digimon_collection c ON c.id=l.companion_id
            WHERE c.owner_guid=? ORDER BY l.companion_id,l.technique_id`,
		`SELECT JSON_OBJECT('companion',e.companion_id,'slot',e.slot,'technique',e.technique_id)
            FROM digimon_equipped e JOIN digimon_collection c ON c.id=e.companion_id
            WHERE c.owner_guid=? ORDER BY e.companion_id,e.slot`,
		`SELECT JSON_OBJECT('slot',slot,'companion',companion_id) FROM digimon_team WHERE owner_guid=? ORDER BY slot`,
	}
	sections := make([][]json.RawMessage, len(queries))
	for i, query := range queries {
		rows, err := db.Query(query, owner)
		if err != nil {
			t.Fatal(err)
		}
		sections[i] = []json.RawMessage{}
		for rows.Next() {
			var row json.RawMessage
			if err := rows.Scan(&row); err != nil {
				rows.Close()
				t.Fatal(err)
			}
			sections[i] = append(sections[i], row)
		}
		err = rows.Err()
		rows.Close()
		if err != nil {
			t.Fatal(err)
		}
	}
	result, err := json.Marshal(sections)
	if err != nil {
		t.Fatal(err)
	}
	return string(result)
}

func saveRestartFixture(t *testing.T, bots []*e2eharness.ScenarioBot) {
	t.Helper()
	if selected := os.Getenv("DIGIMON_RESTART_FROM"); selected != "" && selected != t.Name() {
		return
	}
	path := os.Getenv("DIGIMON_RESTART_FIXTURE")
	if path == "" {
		return
	}
	fixtures := make([]restartFixture, 0, len(bots))
	for _, bot := range bots {
		bot.Save(t)
		fixture := restartFixture{Identity: bot.Ident, Owner: uint32(bot.World.CharGUID())}
		fixture.Snapshot = persistentSnapshot(t, bot.CharDB, fixture.Owner)
		fixtures = append(fixtures, fixture)
	}
	data, err := json.MarshalIndent(fixtures, "", "  ")
	if err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(path, data, 0600); err != nil {
		t.Fatal(err)
	}
}

func TestRestart_PreservedCompanions(t *testing.T) {
	if os.Getenv("DIGIMON_RESTART_PROBE") != "1" {
		t.Skip("separate invocation after an actual world restart")
	}
	path := os.Getenv("DIGIMON_RESTART_FIXTURE")
	data, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	var fixtures []restartFixture
	if err := json.Unmarshal(data, &fixtures); err != nil {
		t.Fatal(err)
	}
	if len(fixtures) != 2 {
		t.Fatalf("expected two retained test accounts, got %d", len(fixtures))
	}
	authDB, charDB := e2eharness.OpenTestDBs(t)
	identities := []e2eharness.BotIdent{fixtures[0].Identity, fixtures[1].Identity}
	sessions := e2eharness.LoginBots(t, identities)
	for i, session := range sessions {
		bot := &e2eharness.ScenarioBot{Session: session, AuthDB: authDB, CharDB: charDB, Ident: identities[i]}
		actor := waitOwned(t, bot, 1)[0]
		var health float64
		var level uint32
		if err := charDB.QueryRow(`SELECT c.level,c.health_pct FROM digimon_active a
            JOIN digimon_collection c ON c.id=a.companion_id WHERE a.owner_guid=?`, fixtures[i].Owner).Scan(&level, &health); err != nil {
			t.Fatal(err)
		}
		maximum := 60 + 25*level
		expected := uint32(math.Round(float64(maximum) * health / 100))
		deadline := time.Now().Add(5 * time.Second)
		for {
			hp, max := bot.UnitHP(actor)
			if max == maximum && hp == expected {
				break
			}
			if time.Now().After(deadline) {
				t.Fatalf("restored actor health=%d/%d, expected %d/%d", hp, max, expected, maximum)
			}
			time.Sleep(25 * time.Millisecond)
		}
		snapshot := persistentSnapshot(t, charDB, fixtures[i].Owner)
		if snapshot != fixtures[i].Snapshot {
			t.Fatalf("persistent state changed across restart: before=%s after=%s", fixtures[i].Snapshot, snapshot)
		}
	}
	t.Log("PASS: both existing accounts reconnected after restart with identical full collection, learned/equipped techniques, active companion, level, XP, health, wallet, cooldown and summon state")
}
