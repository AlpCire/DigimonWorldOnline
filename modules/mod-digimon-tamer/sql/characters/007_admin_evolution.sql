-- SPDX-License-Identifier: GPL-2.0-or-later
-- Configuration only; preserve collections, identities and learned techniques.
ALTER TABLE `digimon_species`
    ADD COLUMN `stage` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    ADD COLUMN `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    ADD CONSTRAINT `digimon_species_stage` CHECK (`stage` BETWEEN 1 AND 4);
UPDATE `digimon_species` SET `enabled` = 1 WHERE `id` = 1;
ALTER TABLE `digimon_techniques` DROP INDEX `spell_id`, ADD INDEX `digimon_spell_lookup` (`spell_id`);

CREATE TABLE `digimon_aliases` (
    `name` VARCHAR(80) NOT NULL PRIMARY KEY,
    `species_id` INT UNSIGNED NOT NULL,
    CONSTRAINT `digimon_alias_species` FOREIGN KEY (`species_id`) REFERENCES `digimon_species` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `digimon_species_techniques` (
    `species_id` INT UNSIGNED NOT NULL,
    `technique_id` INT UNSIGNED NOT NULL,
    PRIMARY KEY (`species_id`, `technique_id`),
    CONSTRAINT `digimon_st_species` FOREIGN KEY (`species_id`) REFERENCES `digimon_species` (`id`),
    CONSTRAINT `digimon_st_technique` FOREIGN KEY (`technique_id`) REFERENCES `digimon_techniques` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `digimon_evolution_rules` (
    `source_species` INT UNSIGNED NOT NULL PRIMARY KEY,
    `target_species` INT UNSIGNED NOT NULL,
    `required_level` TINYINT UNSIGNED NOT NULL,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    CONSTRAINT `digimon_evo_source` FOREIGN KEY (`source_species`) REFERENCES `digimon_species` (`id`),
    CONSTRAINT `digimon_evo_target` FOREIGN KEY (`target_species`) REFERENCES `digimon_species` (`id`),
    CONSTRAINT `digimon_evo_distinct` CHECK (`source_species` <> `target_species`),
    CONSTRAINT `digimon_evo_level` CHECK (`required_level` BETWEEN 2 AND 80)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE `digimon_admin_audit` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `account_id` INT UNSIGNED NOT NULL,
    `owner_guid` INT UNSIGNED NOT NULL,
    `companion_id` INT UNSIGNED NOT NULL,
    `old_species` INT UNSIGNED NOT NULL,
    `new_species` INT UNSIGNED NOT NULL,
    `old_level` TINYINT UNSIGNED NOT NULL,
    `new_level` TINYINT UNSIGNED NOT NULL,
    `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO `digimon_species_techniques` (`species_id`, `technique_id`)
    SELECT `id`, 1 FROM `digimon_species`;
INSERT IGNORE INTO `digimon_species_techniques` (`species_id`, `technique_id`)
    SELECT `id`, 2 FROM `digimon_species`;
-- Names/rule only, disabled until genuine models/templates have been installed.
INSERT IGNORE INTO `digimon_species` (`id`, `creature_entry`, `name`, `stage`, `enabled`) VALUES
    (101, 900130, 'Patamon', 1, 0), (102, 900131, 'Angemon', 2, 0);
INSERT IGNORE INTO `digimon_evolution_rules` (`source_species`, `target_species`, `required_level`, `enabled`)
    VALUES (101, 102, 22, 0);
INSERT IGNORE INTO `digimon_schema_version` (`version`) VALUES (7);
