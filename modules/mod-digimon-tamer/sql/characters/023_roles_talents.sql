-- SPDX-License-Identifier: GPL-2.0-or-later
-- Per-companion group role and level-budgeted talent investment.
ALTER TABLE `digimon_collection`
    ADD COLUMN `combat_role` ENUM('DPS','TANK','HEALER') NOT NULL DEFAULT 'DPS' AFTER `level`;

CREATE TABLE IF NOT EXISTS `digimon_talent_points` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `companion_id` INT UNSIGNED NOT NULL,
    `talent_id` TINYINT UNSIGNED NOT NULL,
    `invested_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    KEY `digimon_talent_companion` (`companion_id`,`talent_id`),
    CONSTRAINT `digimon_talent_owner` FOREIGN KEY (`companion_id`)
        REFERENCES `digimon_collection`(`id`) ON DELETE CASCADE,
    CONSTRAINT `digimon_talent_node` CHECK (`talent_id` BETWEEN 1 AND 3)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO `digimon_techniques`
    (`id`,`name`,`spell_id`,`required_stage`,`price`,`enabled`,`special`,`localized_name`,
     `category`,`min_level`,`cooldown_override_ms`,`damage_type`,`scaling_model`,`effect_note`,`control_limited`)
VALUES (2002,'Healing Pulse',2061,1,0,1,0,'Pulso sanador','MMO_CUSTOM',5,8000,
        'ARCANE','TALENT_HEAL','Cura a un Digimon aliado; requiere cinco puntos en Restauracion',0);

INSERT IGNORE INTO `digimon_species_techniques` (`species_id`,`technique_id`)
    SELECT `id`,2002 FROM `digimon_species`;

INSERT IGNORE INTO `digimon_schema_version`(`version`) VALUES(23);
