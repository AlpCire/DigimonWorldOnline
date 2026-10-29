-- SPDX-License-Identifier: GPL-2.0-or-later
ALTER TABLE `digimon_tamer`
    ADD COLUMN `capture_ready_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    ADD COLUMN `capture_token` CHAR(32) CHARACTER SET `ascii` COLLATE `ascii_bin` NULL;
CREATE TABLE IF NOT EXISTS `digimon_wild_life` (
    `spawn_guid` INT UNSIGNED NOT NULL,
    `map_id` SMALLINT UNSIGNED NOT NULL,
    `instance_id` INT UNSIGNED NOT NULL,
    `life_token` CHAR(32) CHARACTER SET `ascii` COLLATE `ascii_bin` NOT NULL UNIQUE,
    `claimed` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `ready_at` BIGINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`spawn_guid`, `map_id`, `instance_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
INSERT IGNORE INTO `digimon_schema_version` (`version`) VALUES (5);
