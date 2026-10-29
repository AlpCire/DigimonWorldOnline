-- SPDX-License-Identifier: GPL-2.0-or-later
-- Match the internal Impmon training counter to the native forest quests.
-- Keep completed trials and earned catalysts intact for existing companions.

UPDATE digimon_route_requirements SET
 mission_name='La chispa del hechicero', victories_required=8,
 mission_creature_entry=900171
WHERE source_species=103 AND target_species=105;

UPDATE digimon_route_requirements SET
 mission_name='El pacto de las sombras', victories_required=1,
 mission_creature_entry=900174
WHERE source_species=103 AND target_species=108;

UPDATE digimon_route_requirements SET
 mission_name='Voces del otro lado', victories_required=8,
 mission_creature_entry=900171
WHERE source_species=103 AND target_species=112;

UPDATE digimon_route_requirements SET
 mission_name='Aprendiz de la tempestad', victories_required=1,
 mission_creature_entry=900174
WHERE source_species=103 AND target_species=115;

INSERT IGNORE INTO digimon_schema_version(version) VALUES(27);
