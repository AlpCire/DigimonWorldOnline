-- SPDX-License-Identifier: GPL-2.0-or-later
-- Upgrade the initial laboratory constraint without deleting collection rows.
ALTER TABLE `digimon_collection`
    DROP CHECK `digimon_starter_identity`,
    ADD CONSTRAINT `digimon_starter_identity` CHECK (
        (`is_starter` = 1 AND `starter_owner` IS NOT NULL AND `starter_owner` = `owner_guid`)
        OR (`is_starter` = 0 AND `starter_owner` IS NULL));
INSERT IGNORE INTO `digimon_schema_version`(`version`) VALUES (2);
