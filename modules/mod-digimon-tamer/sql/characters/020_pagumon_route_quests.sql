-- SPDX-License-Identifier: GPL-2.0-or-later
-- Pagumon -> Impmon requires the native Pagumon story and its arcane catalyst.
-- The level requirement remains on the evolution edge (level 10).
INSERT INTO digimon_route_requirements
(source_species,target_species,affinity,affinity_required,victories_required,catalyst_required,mission_name,catalyst_item,mission_creature_entry)
VALUES (104,103,'ARCANO',0,0,1,'Un vínculo elegido',900210,0)
ON DUPLICATE KEY UPDATE
affinity=VALUES(affinity),affinity_required=VALUES(affinity_required),
victories_required=VALUES(victories_required),catalyst_required=VALUES(catalyst_required),
mission_name=VALUES(mission_name),catalyst_item=VALUES(catalyst_item),
mission_creature_entry=VALUES(mission_creature_entry);

INSERT IGNORE INTO digimon_schema_version(version) VALUES (20);
