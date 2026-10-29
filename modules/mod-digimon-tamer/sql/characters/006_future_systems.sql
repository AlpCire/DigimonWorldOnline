-- SPDX-License-Identifier: GPL-2.0-or-later
CREATE TABLE IF NOT EXISTS `digimon_team` (
    `owner_guid` INT UNSIGNED NOT NULL,
    `slot` TINYINT UNSIGNED NOT NULL,
    `companion_id` INT UNSIGNED NOT NULL UNIQUE,
    PRIMARY KEY (`owner_guid`, `slot`),
    CONSTRAINT `digimon_team_owned` FOREIGN KEY (`owner_guid`, `companion_id`)
        REFERENCES `digimon_collection`(`owner_guid`, `id`) ON DELETE CASCADE,
    CONSTRAINT `digimon_team_slot` CHECK (`slot` BETWEEN 0 AND 2)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `digimon_evolutions` (
    `id` INT UNSIGNED NOT NULL PRIMARY KEY,
    `species_id` INT UNSIGNED NOT NULL,
    `from_stage` TINYINT UNSIGNED NOT NULL,
    `to_stage` TINYINT UNSIGNED NOT NULL,
    `companion_level` TINYINT UNSIGNED NOT NULL,
    `tamer_level` TINYINT UNSIGNED NOT NULL,
    `price` BIGINT UNSIGNED NOT NULL,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    UNIQUE KEY `digimon_evolution_path` (`species_id`, `from_stage`, `to_stage`),
    CONSTRAINT `digimon_evolution_species` FOREIGN KEY (`species_id`) REFERENCES `digimon_species`(`id`),
    CONSTRAINT `digimon_evolution_stages` CHECK (`from_stage` BETWEEN 1 AND 3 AND `to_stage` = `from_stage` + 1),
    CONSTRAINT `digimon_evolution_levels` CHECK (`companion_level` BETWEEN 1 AND 80 AND `tamer_level` BETWEEN 1 AND 80)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Catalog only: no additional starter, capture or evolution is enabled.
INSERT IGNORE INTO `digimon_species` VALUES
    (2, 900104, 'Oso digital', 0, NULL),
    (3, 900105, 'Felino digital', 0, NULL),
    (4, 900106, 'Rapaz digital', 0, NULL);
-- Placeholder requirements for future design review; runtime has no evolve command.
INSERT IGNORE INTO `digimon_evolutions` VALUES
    (1, 1, 1, 2, 20, 10, 100, 0),
    (2, 1, 2, 3, 40, 30, 300, 0),
    (3, 1, 3, 4, 60, 60, 600, 0);
INSERT IGNORE INTO `digimon_team` (`owner_guid`, `slot`, `companion_id`)
    SELECT `owner_guid`, 0, `companion_id` FROM `digimon_active`;
INSERT IGNORE INTO `digimon_schema_version` (`version`) VALUES (6);
