// SPDX-License-Identifier: GPL-2.0-or-later
#include "DigimonDatabase.h"
#include "MySQLPreparedStatement.h"
#include <algorithm>

namespace Digimon
{
void Connection::DoPrepareStatements()
{
    m_stmts.resize(CountStatements);
    Register(SelectSpecies, "SELECT id, creature_entry, name, starter_enabled, capture_entry, stage, enabled "
                            "FROM digimon_species");
    Register(SelectAliases, "SELECT name, species_id FROM digimon_aliases");
    Register(SelectSpeciesTechniques, "SELECT species_id, technique_id FROM digimon_species_techniques");
    Register(SelectEvolutions, "SELECT source_species, target_species, required_level FROM digimon_evolution_rules "
                               "WHERE enabled = 1");
    Register(SelectTechniques,
             "SELECT id, name, spell_id, required_stage, price, enabled, special FROM digimon_techniques");
    Register(InsertTamer, "INSERT IGNORE INTO digimon_tamer (owner_guid, bits) VALUES (?, ?)");
    Register(SelectTamer,
             "SELECT bits, collection_capacity, summoned, capture_ready_at FROM digimon_tamer WHERE owner_guid = ?");
    Register(SelectCollection, "SELECT id, species_id, stage, level, experience, health_pct FROM digimon_collection "
                               "WHERE owner_guid = ? ORDER BY id");
    Register(InsertStarter, "INSERT INTO digimon_collection (owner_guid, species_id, is_starter, starter_owner, stage) "
                            "SELECT ?, id, 1, ?, stage FROM digimon_species WHERE id = ? AND starter_enabled = 1 "
                            "AND enabled = 1 "
                            "ON DUPLICATE KEY UPDATE id = digimon_collection.id");
    Register(SelectStarter, "SELECT id, species_id FROM digimon_collection WHERE starter_owner = ?");
    Register(InsertInitialActive, "INSERT IGNORE INTO digimon_active (owner_guid, companion_id) "
                                  "SELECT owner_guid, id FROM digimon_collection WHERE starter_owner = ?");
    Register(SelectActive, "SELECT companion_id FROM digimon_active WHERE owner_guid = ?");
    Register(UpdateSummoned, "UPDATE digimon_tamer SET summoned = ? WHERE owner_guid = ?");
    Register(SaveHealth, "UPDATE digimon_collection SET health_pct = ? WHERE owner_guid = ? AND id = ?");
    Register(SelectLearned, "SELECT l.companion_id, l.technique_id FROM digimon_learned l "
                            "JOIN digimon_collection c ON c.id = l.companion_id WHERE c.owner_guid = ?");
    Register(SelectEquipped, "SELECT e.companion_id, e.slot, e.technique_id FROM digimon_equipped e "
                             "JOIN digimon_collection c ON c.id = e.companion_id WHERE c.owner_guid = ?");
    Register(DebitTechnique, "UPDATE digimon_tamer t JOIN digimon_collection c ON c.owner_guid = t.owner_guid "
                             "JOIN digimon_techniques k ON k.id = ? "
                             "JOIN digimon_species_techniques st ON st.species_id = c.species_id "
                             "AND st.technique_id = k.id SET t.bits = t.bits - k.price, t.purchase_serial = "
                             "t.purchase_serial + 1, t.purchase_token = ? "
                             "WHERE t.owner_guid = ? AND c.id = ? AND k.enabled = 1 AND c.stage >= k.required_stage "
                             "AND t.bits >= k.price AND NOT EXISTS (SELECT 1 FROM digimon_learned l "
                             "WHERE l.companion_id = c.id AND l.technique_id = k.id)");
    Register(LearnAfterDebit, "INSERT INTO digimon_learned (companion_id, technique_id) SELECT c.id, ? "
                              "FROM digimon_collection c JOIN digimon_tamer t ON t.owner_guid = c.owner_guid "
                              "WHERE t.owner_guid = ? AND c.id = ? AND t.purchase_token = ?");
    Register(EquipTechnique,
             "INSERT INTO digimon_equipped (companion_id, slot, technique_id) "
             "SELECT c.id, ?, l.technique_id FROM digimon_collection c "
             "JOIN digimon_learned l ON l.companion_id = c.id JOIN digimon_techniques k ON k.id = l.technique_id "
             "JOIN digimon_species_techniques st ON st.species_id = c.species_id AND st.technique_id = k.id "
             "WHERE c.owner_guid = ? AND c.id = ? AND k.id = ? AND k.enabled = 1 AND c.stage >= k.required_stage");
    Register(UnequipTechnique, "DELETE e FROM digimon_equipped e JOIN digimon_collection c ON c.id = e.companion_id "
                               "WHERE c.owner_guid = ? AND c.id = ? AND e.technique_id = ?");
    Register(SaveProgress,
             "UPDATE digimon_collection SET level = ?, experience = ?, health_pct = ?, species_id = ?, stage = ? "
             "WHERE owner_guid = ? AND id = ?");
    Register(PruneEquipment, "DELETE e FROM digimon_equipped e JOIN digimon_collection c ON c.id = e.companion_id "
                              "JOIN digimon_techniques k ON k.id = e.technique_id "
                              "WHERE c.owner_guid = ? AND c.id = ? AND (k.required_stage > c.stage OR NOT EXISTS "
                              "(SELECT 1 FROM digimon_species_techniques st WHERE st.species_id = c.species_id "
                              "AND st.technique_id = k.id))");
    Register(AuditLevel, "INSERT INTO digimon_admin_audit (account_id, owner_guid, companion_id, old_species, "
                          "new_species, old_level, new_level) VALUES (?, ?, ?, ?, ?, ?, ?)");
    Register(RewardBits, "UPDATE digimon_tamer SET bits = bits + ? WHERE owner_guid = ? AND bits <= 1000000000 - ?");
    Register(BeginCaptureAttempt, "UPDATE digimon_tamer SET capture_ready_at = ?, capture_token = ? WHERE owner_guid = "
                                  "? AND capture_ready_at <= ?");
    Register(
        InsertCaptured,
        "INSERT INTO digimon_collection (owner_guid, species_id, capture_key, level, health_pct, stage) "
        "SELECT t.owner_guid, s.id, w.life_token, ?, ?, s.stage FROM digimon_tamer t JOIN digimon_species s ON s.id = ? "
        "JOIN digimon_wild_life w ON w.life_token = ? AND w.claimed = 0 "
        "WHERE t.owner_guid = ? AND t.capture_token = ? AND s.capture_entry = ? AND s.enabled = 1 "
        "AND (SELECT COUNT(*) FROM digimon_collection c WHERE c.owner_guid = t.owner_guid) < t.collection_capacity");
    Register(SelectCaptured, "SELECT id, owner_guid FROM digimon_collection WHERE capture_key = ?");
    Register(ChangeActive,
             "INSERT INTO digimon_active (owner_guid, companion_id) SELECT owner_guid, id FROM digimon_collection "
             "WHERE owner_guid = ? AND id = ? ON DUPLICATE KEY UPDATE companion_id = ?");
    Register(EnsureWildLife,
             "INSERT IGNORE INTO digimon_wild_life (spawn_guid, map_id, instance_id, life_token) VALUES (?, ?, ?, ?)");
    Register(SelectWildLife, "SELECT life_token, claimed, ready_at FROM digimon_wild_life WHERE spawn_guid = ? AND "
                             "map_id = ? AND instance_id = ?");
    Register(ResetWildLife,
             "UPDATE digimon_wild_life SET life_token = ?, claimed = 0, ready_at = 0 "
             "WHERE spawn_guid = ? AND map_id = ? AND instance_id = ? AND (claimed = 0 OR ready_at <= ?)");
    Register(ClaimCapturedWild, "UPDATE digimon_wild_life w JOIN digimon_collection c ON c.capture_key = w.life_token "
                                "SET w.claimed = 1, w.ready_at = ? WHERE w.life_token = ? AND c.owner_guid = ?");
    Register(SelectUnlocks, "SELECT tamer_level, prepared_slots, planned_simultaneous, potion_tier FROM "
                            "digimon_unlocks ORDER BY tamer_level");
    Register(SetTeamLead, "INSERT INTO digimon_team (owner_guid, slot, companion_id) "
                          "SELECT owner_guid, 0, companion_id FROM digimon_active WHERE owner_guid = ? "
                          "ON DUPLICATE KEY UPDATE companion_id = VALUES(companion_id)");
}

void Connection::Register(Statement index, std::string_view sql)
{
    // These module-owned literals contain no quoted question marks.
    _parameters[index] = static_cast<uint8>(std::count(sql.begin(), sql.end(), '?'));
    PrepareStatement(index, sql, CONNECTION_SYNCH);
}

PreparedStatement<Connection>* Connection::MakeStatement(Statement index)
{
    return new PreparedStatement<Connection>(index, _parameters[index]);
}
} // namespace Digimon
