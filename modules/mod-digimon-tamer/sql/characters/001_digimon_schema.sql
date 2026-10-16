-- SPDX-License-Identifier: GPL-2.0-or-later
-- Own tables only. Run with the characters schema selected. Never delete player rows on upgrade.
CREATE TABLE IF NOT EXISTS `digimon_schema_version` (
    `version` INT UNSIGNED NOT NULL PRIMARY KEY,
    `applied_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `digimon_species` (
    `id` INT UNSIGNED NOT NULL PRIMARY KEY,
    `creature_entry` INT UNSIGNED NOT NULL UNIQUE,
    `name` VARCHAR(80) NOT NULL,
    `starter_enabled` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `capture_entry` INT UNSIGNED NULL UNIQUE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `digimon_techniques` (
    `id` INT UNSIGNED NOT NULL PRIMARY KEY,
    `name` VARCHAR(80) NOT NULL,
    `spell_id` INT UNSIGNED NOT NULL UNIQUE,
    `required_stage` TINYINT UNSIGNED NOT NULL,
    `price` BIGINT UNSIGNED NOT NULL,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `special` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    CONSTRAINT `digimon_technique_stage` CHECK (`required_stage` BETWEEN 1 AND 4)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `digimon_tamer` (
    `owner_guid` INT UNSIGNED NOT NULL PRIMARY KEY,
    `bits` BIGINT UNSIGNED NOT NULL DEFAULT 100,
    `collection_capacity` SMALLINT UNSIGNED NOT NULL DEFAULT 30,
    `summoned` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT `digimon_tamer_character` FOREIGN KEY (`owner_guid`) REFERENCES `characters`(`guid`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `digimon_collection` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `owner_guid` INT UNSIGNED NOT NULL,
    `species_id` INT UNSIGNED NOT NULL,
    `is_starter` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `starter_owner` INT UNSIGNED NULL,
    `stage` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `level` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `experience` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    `health_pct` FLOAT NOT NULL DEFAULT 100,
    `capture_key` VARCHAR(100) NULL,
    `obtained_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY `digimon_unique_starter` (`starter_owner`),
    UNIQUE KEY `digimon_capture_once` (`capture_key`),
    UNIQUE KEY `digimon_owner_companion` (`owner_guid`, `id`),
    CONSTRAINT `digimon_collection_owner` FOREIGN KEY (`owner_guid`)
        REFERENCES `digimon_tamer`(`owner_guid`) ON DELETE CASCADE,
    CONSTRAINT `digimon_collection_species` FOREIGN KEY (`species_id`) REFERENCES `digimon_species`(`id`),
    CONSTRAINT `digimon_collection_stage` CHECK (`stage` BETWEEN 1 AND 4),
    CONSTRAINT `digimon_starter_identity` CHECK (
        (`is_starter` = 1 AND `starter_owner` IS NOT NULL AND `starter_owner` = `owner_guid`)
        OR (`is_starter` = 0 AND `starter_owner` IS NULL)),
    CONSTRAINT `digimon_collection_level` CHECK (`level` BETWEEN 1 AND 80),
    CONSTRAINT `digimon_collection_health` CHECK (`health_pct` BETWEEN 0 AND 100)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `digimon_active` (
    `owner_guid` INT UNSIGNED NOT NULL PRIMARY KEY,
    `companion_id` INT UNSIGNED NOT NULL UNIQUE,
    CONSTRAINT `digimon_active_owned` FOREIGN KEY (`owner_guid`, `companion_id`)
        REFERENCES `digimon_collection`(`owner_guid`, `id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `digimon_learned` (
    `companion_id` INT UNSIGNED NOT NULL,
    `technique_id` INT UNSIGNED NOT NULL,
    PRIMARY KEY (`companion_id`, `technique_id`),
    CONSTRAINT `digimon_learned_companion` FOREIGN KEY (`companion_id`)
        REFERENCES `digimon_collection`(`id`) ON DELETE CASCADE,
    CONSTRAINT `digimon_learned_technique` FOREIGN KEY (`technique_id`) REFERENCES `digimon_techniques`(`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `digimon_equipped` (
    `companion_id` INT UNSIGNED NOT NULL,
    `slot` TINYINT UNSIGNED NOT NULL,
    `technique_id` INT UNSIGNED NOT NULL,
    PRIMARY KEY (`companion_id`, `slot`),
    UNIQUE KEY `digimon_equip_once` (`companion_id`, `technique_id`),
    CONSTRAINT `digimon_equipped_learned` FOREIGN KEY (`companion_id`, `technique_id`)
        REFERENCES `digimon_learned`(`companion_id`, `technique_id`) ON DELETE CASCADE,
    CONSTRAINT `digimon_equipped_slot` CHECK (`slot` <= 4)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `digimon_unlocks` (
    `tamer_level` TINYINT UNSIGNED NOT NULL PRIMARY KEY,
    `prepared_slots` TINYINT UNSIGNED NOT NULL,
    `planned_simultaneous` TINYINT UNSIGNED NOT NULL,
    `potion_tier` TINYINT UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO `digimon_species` (`id`, `creature_entry`, `name`, `starter_enabled`, `capture_entry`)
    VALUES (1, 900102, 'Lobo digital', 1, 900103);
INSERT IGNORE INTO `digimon_techniques` VALUES
    (1, 'Mordisco Digital', 17253, 1, 20, 1, 0),
    (2, 'Aullido de Guerra', 24604, 1, 35, 1, 0),
    (3, 'Garra Feroz', 16827, 2, 80, 0, 0),
    (4, 'Colmillo Rompedatos', 52474, 3, 150, 0, 0),
    (5, 'Furia del Núcleo', 19574, 4, 250, 0, 1);
INSERT IGNORE INTO `digimon_unlocks` VALUES
    (1, 1, 1, 0), (10, 1, 1, 1), (20, 2, 1, 1), (30, 2, 1, 2),
    (40, 2, 2, 2), (60, 3, 2, 2), (80, 3, 3, 2);
INSERT IGNORE INTO `digimon_schema_version`(`version`) VALUES (1);
