-- SPDX-License-Identifier: GPL-2.0-or-later
-- Directed evolution graph. The legacy one-edge-per-source table remains intact;
-- this table is the runtime source and safely supports branches and convergences.
CREATE TABLE IF NOT EXISTS `digimon_evolution_edges` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `source_species` INT UNSIGNED NOT NULL,
    `target_species` INT UNSIGNED NOT NULL,
    `required_level` TINYINT UNSIGNED NOT NULL,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `classification` ENUM('CANONICAL', 'GAME_REFERENCE', 'MMO_CUSTOM') NOT NULL DEFAULT 'MMO_CUSTOM',
    `source_note` VARCHAR(255) NOT NULL DEFAULT '',
    `confidence` DECIMAL(4,3) NOT NULL DEFAULT 0.000,
    UNIQUE KEY `digimon_evolution_edge` (`source_species`, `target_species`),
    KEY `digimon_evolution_target` (`target_species`),
    CONSTRAINT `digimon_edge_source` FOREIGN KEY (`source_species`) REFERENCES `digimon_species` (`id`),
    CONSTRAINT `digimon_edge_target` FOREIGN KEY (`target_species`) REFERENCES `digimon_species` (`id`),
    CONSTRAINT `digimon_edge_distinct` CHECK (`source_species` <> `target_species`),
    CONSTRAINT `digimon_edge_level` CHECK (`required_level` BETWEEN 2 AND 85),
    CONSTRAINT `digimon_edge_runtime_cap` CHECK (`enabled` = 0 OR `required_level` <= 80),
    CONSTRAINT `digimon_edge_confidence` CHECK (`confidence` BETWEEN 0 AND 1)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Carry forward only already configured project rules. New design edges stay disabled
-- until their target model, creature template, and requirements are validated.
INSERT IGNORE INTO `digimon_evolution_edges`
    (`source_species`, `target_species`, `required_level`, `enabled`, `classification`, `source_note`, `confidence`)
SELECT `source_species`, `target_species`, `required_level`, `enabled`, 'MMO_CUSTOM',
       'Migrated from the existing project runtime rule; design origin requires review', 0.000
FROM `digimon_evolution_rules`;

INSERT IGNORE INTO `digimon_schema_version` (`version`) VALUES (8);
