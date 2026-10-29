-- SPDX-License-Identifier: GPL-2.0-or-later
-- Replace the three prototype ranks with 27 server-validated tree nodes.
ALTER TABLE `digimon_talent_points`
    DROP CHECK `digimon_talent_node`,
    ADD CONSTRAINT `digimon_talent_node_v2` CHECK (`talent_id` BETWEEN 1 AND 27);

-- Old prototype ranks above five become free points again, never lost levels.
DELETE `p` FROM `digimon_talent_points` `p`
JOIN (
    SELECT `id` FROM (
        SELECT `id`,ROW_NUMBER() OVER
            (PARTITION BY `companion_id`,`talent_id` ORDER BY `id`) AS `ordinal`
        FROM `digimon_talent_points` WHERE `talent_id` IN (1,2,3)
    ) `ranked` WHERE `ordinal` > 5
) `excess` ON `excess`.`id`=`p`.`id`;

INSERT IGNORE INTO `digimon_techniques`
    (`id`,`name`,`spell_id`,`required_stage`,`price`,`enabled`,`special`,`localized_name`,
     `category`,`min_level`,`cooldown_override_ms`,`damage_type`,`scaling_model`,`effect_note`,`control_limited`)
VALUES
    (2000,'Data Bolt',686,1,0,1,0,'Descarga de datos','MMO_CUSTOM',12,8000,
     'SHADOW','TALENT_ACTIVE','Ataque dirigido; requiere Descarga digital',0),
    (2001,'Digital Taunt',355,1,0,1,0,'Provocación digital','MMO_CUSTOM',12,12000,
     'PHYSICAL','TALENT_ACTIVE','Atrae la atención del objetivo; requiere Provocar',0),
    (2003,'Data Storm',421,1,0,1,0,'Tormenta de datos','MMO_CUSTOM',22,20000,
     'NATURE','TALENT_ACTIVE','Descarga encadenada; requiere Tormenta de datos',0),
    (2004,'Data Barrier',17,1,0,1,0,'Barrera de datos','MMO_CUSTOM',22,20000,
     'ARCANE','TALENT_ACTIVE','Escudo a un Digimon aliado; requiere Barrera digital o de red',0),
    (2005,'Emergency Heal',2061,1,0,1,0,'Sanación de emergencia','MMO_CUSTOM',12,15000,
     'ARCANE','TALENT_ACTIVE','Cura potente; requiere Sanación de emergencia',0);

CREATE TABLE IF NOT EXISTS `digimon_talent_unlocks` (
    `technique_id` INT UNSIGNED NOT NULL,
    `talent_id` TINYINT UNSIGNED NOT NULL,
    `required_rank` TINYINT UNSIGNED NOT NULL,
    PRIMARY KEY (`technique_id`,`talent_id`),
    CONSTRAINT `digimon_talent_unlock_technique` FOREIGN KEY (`technique_id`)
        REFERENCES `digimon_techniques`(`id`) ON DELETE CASCADE,
    CONSTRAINT `digimon_talent_unlock_node` CHECK (`talent_id` BETWEEN 1 AND 27),
    CONSTRAINT `digimon_talent_unlock_rank` CHECK (`required_rank` BETWEEN 1 AND 5)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO `digimon_talent_unlocks` (`technique_id`,`talent_id`,`required_rank`)
VALUES (2000,9,1),(2001,17,1),(2002,3,5),(2003,11,1),(2004,19,1),(2004,27,1),(2005,25,1);

INSERT IGNORE INTO `digimon_species_techniques` (`species_id`,`technique_id`)
    SELECT `s`.`id`,`k`.`id` FROM `digimon_species` `s`
    JOIN `digimon_techniques` `k` ON `k`.`id` IN (2000,2001,2003,2004,2005);

INSERT IGNORE INTO `digimon_schema_version`(`version`) VALUES(24);
