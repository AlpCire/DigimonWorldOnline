-- SPDX-License-Identifier: GPL-2.0-or-later
-- Capture only the two tested, private client models used by the first forest adventure.
-- Preserve any deliberate administrator capture mapping.
UPDATE digimon_species SET capture_entry=900170
WHERE id=101 AND capture_entry IS NULL AND enabled=1;
UPDATE digimon_species SET capture_entry=900171
WHERE id=104 AND capture_entry IS NULL AND enabled=1;
INSERT IGNORE INTO digimon_schema_version(version) VALUES (26);
