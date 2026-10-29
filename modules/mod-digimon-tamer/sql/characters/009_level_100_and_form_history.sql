-- SPDX-License-Identifier: GPL-2.0-or-later
-- Raise companion progression to 100 and persist discovered forms/history.
-- Existing collections and evolution edges are retained.
ALTER TABLE `digimon_collection`
    DROP CHECK `digimon_collection_level`,
    ADD CONSTRAINT `digimon_collection_level` CHECK (`level` BETWEEN 1 AND 100);
ALTER TABLE `digimon_evolution_edges`
    DROP CHECK `digimon_edge_level`,
    DROP CHECK `digimon_edge_runtime_cap`,
    ADD CONSTRAINT `digimon_edge_level` CHECK (`required_level` BETWEEN 2 AND 100),
    ADD CONSTRAINT `digimon_edge_runtime_cap` CHECK (`enabled` = 0 OR `required_level` <= 100);
ALTER TABLE `digimon_evolution_rules`
    DROP CHECK `digimon_evo_level`,
    ADD CONSTRAINT `digimon_evo_level` CHECK (`required_level` BETWEEN 2 AND 100);
ALTER TABLE `digimon_evolutions`
    DROP CHECK `digimon_evolution_levels`,
    ADD CONSTRAINT `digimon_evolution_levels` CHECK (`companion_level` BETWEEN 1 AND 100 AND `tamer_level` BETWEEN 1 AND 80);

CREATE TABLE IF NOT EXISTS `digimon_companion_forms` (
    `companion_id` INT UNSIGNED NOT NULL,
    `species_id` INT UNSIGNED NOT NULL,
    `first_unlocked_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`companion_id`, `species_id`),
    KEY `digimon_forms_species` (`species_id`),
    CONSTRAINT `digimon_forms_companion` FOREIGN KEY (`companion_id`) REFERENCES `digimon_collection` (`id`) ON DELETE CASCADE,
    CONSTRAINT `digimon_forms_species_fk` FOREIGN KEY (`species_id`) REFERENCES `digimon_species` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `digimon_evolution_history` (
    `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `companion_id` INT UNSIGNED NOT NULL,
    `from_species` INT UNSIGNED NOT NULL,
    `to_species` INT UNSIGNED NOT NULL,
    `level` TINYINT UNSIGNED NOT NULL,
    `action` ENUM('EVOLVE', 'DEVOLVE', 'REUSE') NOT NULL,
    `changed_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    KEY `digimon_history_companion` (`companion_id`, `id`),
    CONSTRAINT `digimon_history_companion_fk` FOREIGN KEY (`companion_id`) REFERENCES `digimon_collection` (`id`) ON DELETE CASCADE,
    CONSTRAINT `digimon_history_from_fk` FOREIGN KEY (`from_species`) REFERENCES `digimon_species` (`id`),
    CONSTRAINT `digimon_history_to_fk` FOREIGN KEY (`to_species`) REFERENCES `digimon_species` (`id`),
    CONSTRAINT `digimon_history_level` CHECK (`level` BETWEEN 1 AND 100),
    CONSTRAINT `digimon_history_distinct` CHECK (`from_species` <> `to_species`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO `digimon_companion_forms` (`companion_id`, `species_id`)
    SELECT `id`, `species_id` FROM `digimon_collection`;
INSERT IGNORE INTO `digimon_schema_version` (`version`) VALUES (9);
