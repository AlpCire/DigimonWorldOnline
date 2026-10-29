-- SPDX-License-Identifier: GPL-2.0-or-later
-- Impmon uses its own level-gated technique catalog; remove inherited Wolf attacks.
DELETE FROM digimon_species_techniques
WHERE species_id = 103 AND technique_id IN (1, 2);

INSERT IGNORE INTO digimon_schema_version(version) VALUES(14);
