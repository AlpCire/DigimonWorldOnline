-- The introduction offers only first forms whose client models are already installed.
-- Pagumon starts the Impmon branch; Patamon starts the Patamon/Angemon branch.
UPDATE digimon_species
SET starter_enabled=1, enabled=1
WHERE id IN (101,104) AND creature_entry IN (900130,900133);

-- Pagumon reaches the already validated Impmon form at level 10.
UPDATE digimon_evolution_edges
SET enabled=1
WHERE source_species=104 AND target_species=103 AND required_level=10;

INSERT IGNORE INTO digimon_schema_version(version) VALUES (19);
