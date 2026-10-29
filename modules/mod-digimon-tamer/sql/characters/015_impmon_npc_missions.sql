-- SPDX-License-Identifier: GPL-2.0-or-later
-- Impmon's four first-stage routes are offered by the World Digital Guide.
ALTER TABLE digimon_route_requirements
    ADD COLUMN mission_creature_entry INT UNSIGNED NOT NULL DEFAULT 0 AFTER catalyst_item;

UPDATE digimon_route_requirements
SET mission_name='La caza de los lobos', victories_required=40,
    catalyst_required=1, mission_creature_entry=299
WHERE source_species=103 AND target_species IN (105,108,112,115);

INSERT IGNORE INTO digimon_schema_version(version) VALUES(15);
