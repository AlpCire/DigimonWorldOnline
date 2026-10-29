-- SPDX-License-Identifier: GPL-2.0-or-later
-- One specialization and one role per current form. New Patamon branches can be
-- registered later without changing existing Digimon or their saved loadouts.
CREATE TABLE IF NOT EXISTS `digimon_form_specializations` (
    `species_id` INT UNSIGNED NOT NULL PRIMARY KEY,
    `combat_role` ENUM('DPS','TANK','HEALER') NOT NULL,
    `profile_code` VARCHAR(24) NOT NULL,
    `specialization_name` VARCHAR(80) NOT NULL,
    CONSTRAINT `digimon_form_specialization_species` FOREIGN KEY (`species_id`)
        REFERENCES `digimon_species`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `digimon_form_specializations`
    (`species_id`,`combat_role`,`profile_code`,`specialization_name`)
VALUES
    (1,'DPS','BASIC','Instinto lupino'),
    (2,'DPS','BASIC','Instinto salvaje'),
    (3,'DPS','BASIC','Instinto felino'),
    (4,'DPS','BASIC','Instinto rapaz'),
    (101,'DPS','BASIC','Compañero alado'),
    (102,'HEALER','ANGEL','Luz sagrada'),
    (103,'DPS','IMPMON','Travesura'),
    (104,'DPS','BASIC','Datos inquietos'),
    (105,'DPS','ARCANE','Hechicería'),
    (106,'DPS','GUN','Tirador del abismo'),
    (107,'DPS','GUN','Artillería infernal'),
    (108,'DPS','SHADOW','Oscuridad'),
    (109,'DPS','SHADOW','Sangre oscura'),
    (110,'DPS','SHADOW','Pesadilla'),
    (111,'DPS','SHADOW','Soberano oscuro'),
    (112,'DPS','SPECTRAL','Espectro'),
    (113,'DPS','SPECTRAL','Segador'),
    (114,'DPS','SPECTRAL','Ilusionista'),
    (115,'DPS','HEX','Brujería'),
    (116,'DPS','HEX','Maldición'),
    (117,'DPS','HEX','Tentación')
ON DUPLICATE KEY UPDATE
    `combat_role`=VALUES(`combat_role`),
    `profile_code`=VALUES(`profile_code`),
    `specialization_name`=VALUES(`specialization_name`);

-- Existing investments become the loadout of the form currently owned.
-- No point is copied to other forms, so previous passive bonuses cannot stack.
ALTER TABLE `digimon_talent_points`
    ADD COLUMN `species_id` INT UNSIGNED NULL AFTER `companion_id`;
UPDATE `digimon_talent_points` `p`
JOIN `digimon_collection` `c` ON `c`.`id`=`p`.`companion_id`
SET `p`.`species_id`=`c`.`species_id`;
ALTER TABLE `digimon_talent_points`
    MODIFY COLUMN `species_id` INT UNSIGNED NOT NULL,
    ADD KEY `digimon_talent_form` (`companion_id`,`species_id`,`talent_id`),
    ADD CONSTRAINT `digimon_talent_species` FOREIGN KEY (`species_id`)
        REFERENCES `digimon_species`(`id`);

-- Equipped techniques also remember their form. Learned techniques remain
-- companion-wide, but the active form and its talents gate actual use.
ALTER TABLE `digimon_equipped`
    ADD COLUMN `species_id` INT UNSIGNED NULL AFTER `companion_id`;
UPDATE `digimon_equipped` `e`
JOIN `digimon_collection` `c` ON `c`.`id`=`e`.`companion_id`
SET `e`.`species_id`=`c`.`species_id`;
ALTER TABLE `digimon_equipped`
    MODIFY COLUMN `species_id` INT UNSIGNED NOT NULL,
    DROP PRIMARY KEY,
    DROP INDEX `digimon_equip_once`,
    ADD PRIMARY KEY (`companion_id`,`species_id`,`slot`),
    ADD UNIQUE KEY `digimon_equip_once` (`companion_id`,`species_id`,`technique_id`),
    ADD KEY `digimon_equipped_learned_lookup` (`companion_id`,`technique_id`),
    ADD CONSTRAINT `digimon_equipped_species` FOREIGN KEY (`species_id`)
        REFERENCES `digimon_species`(`id`);

UPDATE `digimon_collection` `c`
JOIN `digimon_form_specializations` `f` ON `f`.`species_id`=`c`.`species_id`
SET `c`.`combat_role`=`f`.`combat_role`;

INSERT IGNORE INTO `digimon_schema_version`(`version`) VALUES(25);
