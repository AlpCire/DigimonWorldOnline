-- SPDX-License-Identifier: GPL-2.0-or-later
-- Native quest objectives for every Impmon evolution route. The exact creature
-- objective is intentionally independent of creature XP; quest XP is awarded
-- to the Tamer by AzerothCore when the native quest is turned in.
UPDATE digimon_route_requirements
SET mission_creature_entry=705
WHERE source_species BETWEEN 103 AND 116 AND target_species BETWEEN 105 AND 117;
UPDATE digimon_route_requirements SET mission_creature_entry=16952 WHERE source_species=105 AND target_species=106;
UPDATE digimon_route_requirements SET mission_creature_entry=31812 WHERE source_species=106 AND target_species=107;
UPDATE digimon_route_requirements SET mission_creature_entry=6126 WHERE source_species=108 AND target_species=109;
UPDATE digimon_route_requirements SET mission_creature_entry=21869 WHERE source_species=109 AND target_species=110;
UPDATE digimon_route_requirements SET mission_creature_entry=31812 WHERE source_species=110 AND target_species=111;
UPDATE digimon_route_requirements SET mission_creature_entry=5263 WHERE source_species=112 AND target_species=113;
UPDATE digimon_route_requirements SET mission_creature_entry=22045 WHERE source_species=113 AND target_species=114;
UPDATE digimon_route_requirements SET mission_creature_entry=6127 WHERE source_species=115 AND target_species=116;
UPDATE digimon_route_requirements SET mission_creature_entry=19853 WHERE source_species=116 AND target_species=117;

INSERT IGNORE INTO digimon_schema_version(version) VALUES(16);
