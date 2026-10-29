// SPDX-License-Identifier: GPL-2.0-or-later
#include "DigimonDatabase.h"
#include "MySQLPreparedStatement.h"
#include <algorithm>

namespace Digimon
{
void Connection::DoPrepareStatements()
{
    m_stmts.resize(CountStatements);
    Register(SelectSpentCatalysts, "SELECT item_guid FROM digimon_catalyst_spent WHERE owner_guid=?");
    Register(SpendPhysicalCatalyst, "INSERT INTO digimon_catalyst_spent(item_guid,owner_guid,companion_id,target_species) VALUES(?,?,?,?)");
    Register(CreditBond, "INSERT INTO digimon_bond(companion_id,points,last_care_at) VALUES(?,?,0) ON DUPLICATE KEY UPDATE points=LEAST(points+VALUES(points),100)");
    Register(CreditCareBond, "INSERT INTO digimon_bond(companion_id,points,last_care_at) VALUES(?,1,UNIX_TIMESTAMP()) ON DUPLICATE KEY UPDATE points=IF(last_care_at<=UNIX_TIMESTAMP()-86400,LEAST(points+1,100),points),last_care_at=IF(last_care_at<=UNIX_TIMESTAMP()-86400,UNIX_TIMESTAMP(),last_care_at)");
    Register(SelectConsumables, "SELECT item_entry,kind,happiness_gain,magnitude,duration_seconds,spell_id,enabled FROM digimon_consumables");
    Register(SaveCareAndEffects, "UPDATE digimon_collection SET happiness=?,damage_buff_pct=?,damage_buff_expires=?,protection_buff_pct=?,protection_buff_expires=? WHERE owner_guid=? AND id=?");
    Register(RecordConsumedItem, "INSERT INTO digimon_consumable_spent(item_guid,owner_guid,item_entry) VALUES(?,?,?)");
    Register(SelectConsumedItems, "SELECT item_guid,item_entry FROM digimon_consumable_spent WHERE owner_guid=?");
    Register(DeleteConsumedItem, "DELETE FROM digimon_consumable_spent WHERE item_guid=? AND owner_guid=?");
    Register(SelectStorage, "SELECT c.id,c.species_id,c.stage,c.level,c.experience,s.name,COALESCE((SELECT 1 FROM digimon_active a WHERE a.owner_guid=c.owner_guid AND a.companion_id=c.id),0) "
            "FROM digimon_collection c JOIN digimon_species s ON s.id=c.species_id "
            "WHERE c.owner_guid=? AND NOT EXISTS (SELECT 1 FROM digimon_party p WHERE p.companion_id=c.id) "
            "ORDER BY c.id LIMIT 15 OFFSET ?");
    Register(BuyStorageBox, "UPDATE digimon_tamer SET storage_boxes=storage_boxes+1 WHERE owner_guid=? AND storage_boxes=? AND storage_boxes<100");
    Register(AssignCapturedParty, "INSERT INTO digimon_party(owner_guid,slot,companion_id) "
            "SELECT c.owner_guid,slots.slot,c.id FROM digimon_collection c "
            "JOIN (SELECT 0 AS slot UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5) slots "
            "WHERE c.capture_key=? AND (SELECT COUNT(*) FROM digimon_party p WHERE p.owner_guid=c.owner_guid)<6 "
            "AND NOT EXISTS(SELECT 1 FROM digimon_party p WHERE p.owner_guid=c.owner_guid AND p.slot=slots.slot) "
            "ORDER BY slots.slot LIMIT 1");
    Register(SetPartyLead, "INSERT IGNORE INTO digimon_party(owner_guid,slot,companion_id) "
            "SELECT owner_guid,0,companion_id FROM digimon_active WHERE owner_guid=?");
    Register(MoveToParty, "INSERT INTO digimon_party(owner_guid,slot,companion_id) "
            "SELECT c.owner_guid,slots.slot,c.id FROM digimon_collection c "
            "JOIN (SELECT 0 AS slot UNION ALL SELECT 1 UNION ALL SELECT 2 UNION ALL SELECT 3 UNION ALL SELECT 4 UNION ALL SELECT 5) slots "
            "WHERE c.owner_guid=? AND c.id=? AND (SELECT COUNT(*) FROM digimon_party p WHERE p.owner_guid=c.owner_guid)<6 "
            "AND NOT EXISTS(SELECT 1 FROM digimon_party p WHERE p.owner_guid=c.owner_guid AND p.slot=slots.slot) "
            "AND (SELECT COUNT(*) FROM digimon_collection sc WHERE sc.owner_guid=c.owner_guid AND NOT EXISTS(SELECT 1 FROM digimon_party sp WHERE sp.companion_id=sc.id)) <= "
            "(SELECT storage_boxes*15 FROM digimon_tamer WHERE owner_guid=c.owner_guid) ORDER BY slots.slot LIMIT 1");
    Register(StoreFromParty, "DELETE FROM digimon_party WHERE owner_guid=? AND companion_id=?");
    Register(CountStorage, "SELECT COUNT(*) FROM digimon_collection c WHERE c.owner_guid=? AND NOT EXISTS(SELECT 1 FROM digimon_party p WHERE p.companion_id=c.id)");
    Register(SelectTalentPoints, "SELECT p.companion_id,p.talent_id FROM digimon_talent_points p "
            "JOIN digimon_collection c ON c.id=p.companion_id "
            "WHERE c.owner_guid=? AND p.species_id=c.species_id ORDER BY p.id");
    Register(SpendTalentPoint, "INSERT INTO digimon_talent_points(companion_id,species_id,talent_id) "
            "SELECT c.id,c.species_id,? FROM digimon_collection c WHERE c.owner_guid=? AND c.id=? "
            "AND c.level>1+(SELECT COUNT(*) FROM digimon_talent_points p "
            "WHERE p.companion_id=c.id AND p.species_id=c.species_id) "
            "AND (SELECT COUNT(*) FROM digimon_talent_points p WHERE p.companion_id=c.id "
            "AND p.species_id=c.species_id AND p.talent_id=?)<?");
    Register(ClearTalentPoints, "DELETE p FROM digimon_talent_points p "
            "JOIN digimon_collection c ON c.id=p.companion_id WHERE c.owner_guid=? AND c.id=? "
            "AND p.species_id=c.species_id");
    Register(PruneTalentPoints, "DELETE p FROM digimon_talent_points p JOIN ("
            "SELECT id FROM (SELECT id,ROW_NUMBER() OVER "
            "(PARTITION BY species_id ORDER BY id) AS ordinal FROM digimon_talent_points "
            "WHERE companion_id=?) ranked WHERE ordinal>?) excess ON excess.id=p.id");
    Register(SelectCombatProfiles, "SELECT technique_id,mode,primary_spell,secondary_spell,utility_spell,hits,"
             "duration_ms,range_yards,radius_yards,base_value,per_level,weapon_factor,focus_cost FROM digimon_combat_profiles");
    Register(SelectRequirements, "SELECT source_species,target_species,affinity,affinity_required,victories_required,catalyst_required,mission_name,catalyst_item,mission_creature_entry FROM digimon_route_requirements");
    Register(SelectTrial, "SELECT tr.victories,tr.completed,tr.catalysts,COALESCE(a.points,0) FROM digimon_trials tr LEFT JOIN digimon_affinities a ON a.companion_id=tr.companion_id AND a.affinity=? WHERE tr.companion_id=? AND tr.target_species=?");
    Register(SelectTraining, "SELECT target_species,rewarded_at FROM digimon_training WHERE companion_id=?");
    Register(StartTrial, "INSERT IGNORE INTO digimon_trials(companion_id,target_species) VALUES(?,?)");
    Register(SetTraining, "INSERT INTO digimon_training(companion_id,target_species) VALUES(?,?) ON DUPLICATE KEY UPDATE target_species=VALUES(target_species)");
    Register(CreditTrial, "UPDATE digimon_trials SET victories=LEAST(victories+1,?),catalysts=catalysts+IF(completed=0 AND victories>=?,1,0),completed=IF(victories>=?,1,completed) WHERE companion_id=? AND target_species=?");
    Register(CreditAffinity, "INSERT INTO digimon_affinities(companion_id,affinity,points) VALUES(?,?,1) ON DUPLICATE KEY UPDATE points=LEAST(points+1,100)");
    Register(CompleteNativeTrial, "UPDATE digimon_trials SET victories=GREATEST(victories,?),completed=1,catalysts=0 WHERE companion_id=? AND target_species=?");
    Register(ClearTraining, "DELETE FROM digimon_training WHERE companion_id=?");
    Register(SelectNativeQuest, "SELECT companion_id,target_species FROM digimon_native_quests WHERE owner_guid=? AND quest_id=?");
    Register(SaveNativeQuest, "INSERT INTO digimon_native_quests(owner_guid,quest_id,companion_id,target_species) VALUES(?,?,?,?) ON DUPLICATE KEY UPDATE companion_id=VALUES(companion_id),target_species=VALUES(target_species)");
    Register(DeleteNativeQuest, "DELETE FROM digimon_native_quests WHERE owner_guid=? AND quest_id=?");
    Register(StampTraining, "UPDATE digimon_training SET rewarded_at=? WHERE companion_id=?");
    Register(ConsumeCatalyst, "UPDATE digimon_trials SET catalysts=catalysts-? WHERE companion_id=? AND target_species=? AND catalysts>=?");

    Register(SelectSpecies, "SELECT id, creature_entry, name, starter_enabled, capture_entry, stage, enabled "
                            "FROM digimon_species");
    Register(SelectAliases, "SELECT name, species_id FROM digimon_aliases");
    Register(SelectSpeciesTechniques, "SELECT species_id, technique_id FROM digimon_species_techniques");
    Register(SelectEvolutions, "SELECT source_species, target_species, required_level FROM digimon_evolution_edges "
                               "WHERE enabled = 1 ORDER BY source_species, id");
    Register(SelectTechniques,
             "SELECT id, name, localized_name, spell_id, required_stage, price, enabled, special, min_level, "
             "cooldown_override_ms, control_limited FROM digimon_techniques");
    Register(LearnEligibleTechniques, "INSERT IGNORE INTO digimon_learned (companion_id,technique_id) "
            "SELECT c.id,k.id FROM digimon_collection c JOIN digimon_techniques k "
            "LEFT JOIN digimon_species_techniques st ON st.species_id=c.species_id AND st.technique_id=k.id "
            "WHERE st.technique_id IS NOT NULL AND c.id=? AND c.level>=k.min_level "
            "AND c.stage>=k.required_stage AND k.enabled=1 "
            "AND (NOT EXISTS (SELECT 1 FROM digimon_talent_unlocks u WHERE u.technique_id=k.id) OR EXISTS (SELECT 1 FROM digimon_talent_unlocks u WHERE u.technique_id=k.id AND (SELECT COUNT(*) FROM digimon_talent_points p WHERE p.companion_id=c.id AND p.species_id=c.species_id AND p.talent_id=u.talent_id)>=u.required_rank)) ");
    Register(InsertTamer, "INSERT IGNORE INTO digimon_tamer (owner_guid, bits) VALUES (?, ?)");
    Register(SelectTamer,
             "SELECT bits, collection_capacity, summoned, capture_ready_at, storage_boxes FROM digimon_tamer WHERE owner_guid = ?");
    Register(SelectCollection, "SELECT c.id, c.species_id, c.stage, c.level, c.experience, c.health_pct, COALESCE(b.points,0), "
                               "c.happiness,c.damage_buff_pct,c.damage_buff_expires,c.protection_buff_pct,"
                               "c.protection_buff_expires,COALESCE(p.slot,255),"
                               "COALESCE(f.combat_role,'DPS'),COALESCE(f.profile_code,'BASIC') "
                               "FROM digimon_collection c LEFT JOIN digimon_bond b ON b.companion_id=c.id "
                               "LEFT JOIN digimon_party p ON p.companion_id=c.id "
                               "LEFT JOIN digimon_form_specializations f ON f.species_id=c.species_id "
                               "WHERE c.owner_guid = ? ORDER BY c.id");
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
                             "JOIN digimon_collection c ON c.id=e.companion_id "
                             "WHERE c.owner_guid=? AND e.species_id=c.species_id");
    Register(DebitTechnique, "UPDATE digimon_tamer t JOIN digimon_collection c ON c.owner_guid = t.owner_guid "
                             "JOIN digimon_techniques k ON k.id = ? "
                             "JOIN digimon_species_techniques st ON st.species_id = c.species_id "
                             "AND st.technique_id = k.id SET t.bits = t.bits - k.price, t.purchase_serial = "
                             "t.purchase_serial + 1, t.purchase_token = ? "
                             "WHERE t.owner_guid = ? AND c.id = ? AND k.enabled = 1 AND c.stage >= k.required_stage "
                             "AND (NOT EXISTS (SELECT 1 FROM digimon_talent_unlocks u WHERE u.technique_id=k.id) OR EXISTS (SELECT 1 FROM digimon_talent_unlocks u WHERE u.technique_id=k.id AND (SELECT COUNT(*) FROM digimon_talent_points p WHERE p.companion_id=c.id AND p.species_id=c.species_id AND p.talent_id=u.talent_id)>=u.required_rank)) "
                             "AND c.level >= k.min_level AND t.bits >= k.price AND NOT EXISTS (SELECT 1 FROM digimon_learned l "
                             "WHERE l.companion_id = c.id AND l.technique_id = k.id)");
    Register(LearnAfterDebit, "INSERT INTO digimon_learned (companion_id, technique_id) SELECT c.id, ? "
                              "FROM digimon_collection c JOIN digimon_tamer t ON t.owner_guid = c.owner_guid "
                              "WHERE t.owner_guid = ? AND c.id = ? AND t.purchase_token = ?");
    Register(EquipTechnique,
             "INSERT INTO digimon_equipped (companion_id, species_id, slot, technique_id) "
             "SELECT c.id, c.species_id, ?, l.technique_id FROM digimon_collection c "
             "JOIN digimon_learned l ON l.companion_id = c.id JOIN digimon_techniques k ON k.id = l.technique_id "
             "LEFT JOIN digimon_species_techniques st ON st.species_id = c.species_id AND st.technique_id = k.id "
             "WHERE st.technique_id IS NOT NULL AND c.owner_guid = ? AND c.id = ? "
             "AND k.id = ? AND k.enabled = 1 AND c.stage >= k.required_stage AND c.level >= k.min_level "
             "AND (NOT EXISTS (SELECT 1 FROM digimon_talent_unlocks u WHERE u.technique_id=k.id) OR EXISTS (SELECT 1 FROM digimon_talent_unlocks u WHERE u.technique_id=k.id AND (SELECT COUNT(*) FROM digimon_talent_points p WHERE p.companion_id=c.id AND p.species_id=c.species_id AND p.talent_id=u.talent_id)>=u.required_rank)) ");
    Register(UnequipTechnique, "DELETE e FROM digimon_equipped e JOIN digimon_collection c ON c.id=e.companion_id "
                               "WHERE c.owner_guid=? AND c.id=? AND e.species_id=c.species_id "
                               "AND e.technique_id=?");
    Register(CloneFormEquipment,
             "INSERT IGNORE INTO digimon_equipped (companion_id,species_id,slot,technique_id) "
             "SELECT c.id,c.species_id,e.slot,e.technique_id FROM digimon_collection c "
             "JOIN digimon_equipped e ON e.companion_id=c.id AND e.species_id=? "
             "JOIN digimon_techniques k ON k.id=e.technique_id "
             "JOIN digimon_species_techniques st ON st.species_id=c.species_id "
             "AND st.technique_id=e.technique_id WHERE c.owner_guid=? AND c.id=? "
             "AND k.enabled=1 AND k.required_stage<=c.stage AND k.min_level<=c.level "
             "AND ((e.slot<3 AND k.special=0) OR (e.slot=3 AND c.stage>=2 AND k.special=0) "
             "OR (e.slot=4 AND c.stage=4 AND k.special=1)) "
             "AND (NOT EXISTS (SELECT 1 FROM digimon_talent_unlocks u WHERE u.technique_id=k.id) "
             "OR EXISTS (SELECT 1 FROM digimon_talent_unlocks u WHERE u.technique_id=k.id "
             "AND (SELECT COUNT(*) FROM digimon_talent_points p WHERE p.companion_id=c.id "
             "AND p.species_id=c.species_id AND p.talent_id=u.talent_id)>=u.required_rank))");
    Register(SaveProgress,
             "UPDATE digimon_collection c JOIN digimon_form_specializations f ON f.species_id=? "
             "SET c.level=?,c.experience=?,c.health_pct=?,c.species_id=f.species_id,c.stage=?,"
             "c.combat_role=f.combat_role WHERE c.owner_guid=? AND c.id=?");
    Register(PruneEquipment, "DELETE e FROM digimon_equipped e "
                              "JOIN digimon_collection c ON c.id=e.companion_id "
                              "JOIN digimon_species s ON s.id=e.species_id "
                              "JOIN digimon_techniques k ON k.id=e.technique_id "
                              "WHERE c.owner_guid=? AND c.id=? AND (k.required_stage>s.stage OR "
                              "k.min_level>c.level OR NOT EXISTS "
                              "(SELECT 1 FROM digimon_species_techniques st WHERE st.species_id=e.species_id "
                              "AND st.technique_id=k.id) OR (EXISTS "
                              "(SELECT 1 FROM digimon_talent_unlocks u WHERE u.technique_id=k.id) AND NOT EXISTS "
                              "(SELECT 1 FROM digimon_talent_unlocks u WHERE u.technique_id=k.id "
                              "AND (SELECT COUNT(*) FROM digimon_talent_points p WHERE p.companion_id=c.id "
                              "AND p.species_id=e.species_id AND p.talent_id=u.talent_id)>=u.required_rank)))");
    Register(AuditLevel, "INSERT INTO digimon_admin_audit (account_id, owner_guid, companion_id, old_species, "
                          "new_species, old_level, new_level) VALUES (?, ?, ?, ?, ?, ?, ?)");
    Register(EnsureForms, "INSERT IGNORE INTO digimon_companion_forms (companion_id, species_id) "
                          "SELECT id, species_id FROM digimon_collection WHERE owner_guid = ?");
    Register(SelectDiscoveredForms, "SELECT f.companion_id, f.species_id FROM digimon_companion_forms f "
                                    "JOIN digimon_collection c ON c.id = f.companion_id WHERE c.owner_guid = ?");
    Register(UnlockForm, "INSERT IGNORE INTO digimon_companion_forms (companion_id, species_id) VALUES (?, ?)");
    Register(RecordEvolutionHistory, "INSERT INTO digimon_evolution_history "
                                    "(companion_id, from_species, to_species, level, action) VALUES (?, ?, ?, ?, ?)");
    Register(RewardBits, "UPDATE digimon_tamer SET bits = bits + ? WHERE owner_guid = ? AND bits <= 1000000000 - ?");
    Register(BeginCaptureAttempt, "UPDATE digimon_tamer SET capture_ready_at = ?, capture_token = ? WHERE owner_guid = "
                                  "? AND capture_ready_at <= ?");
    Register(
        InsertCaptured,
        "INSERT INTO digimon_collection (owner_guid, species_id, capture_key, level, health_pct, stage) "
        "SELECT t.owner_guid, s.id, w.life_token, ?, ?, s.stage FROM digimon_tamer t JOIN digimon_species s ON s.id = ? "
        "JOIN digimon_wild_life w ON w.life_token = ? AND w.claimed = 0 "
        "WHERE t.owner_guid = ? AND t.capture_token = ? AND s.capture_entry = ? AND s.enabled = 1 "
        "AND ((SELECT COUNT(*) FROM digimon_party p WHERE p.owner_guid=t.owner_guid)<6 OR "
        "(SELECT COUNT(*) FROM digimon_collection c LEFT JOIN digimon_party p ON p.companion_id=c.id WHERE c.owner_guid=t.owner_guid AND p.companion_id IS NULL)<(t.storage_boxes*15))");
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
