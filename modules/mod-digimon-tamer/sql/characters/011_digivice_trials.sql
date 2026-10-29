-- SPDX-License-Identifier: GPL-2.0-or-later
-- Digivice trials: server-owned progress and digital catalysts, atomic with form unlocks.
CREATE TABLE IF NOT EXISTS digimon_route_requirements (
 source_species INT UNSIGNED NOT NULL, target_species INT UNSIGNED NOT NULL,
 affinity VARCHAR(24) NOT NULL, affinity_required SMALLINT UNSIGNED NOT NULL,
 victories_required SMALLINT UNSIGNED NOT NULL, catalyst_required TINYINT UNSIGNED NOT NULL DEFAULT 1,
 mission_name VARCHAR(80) NOT NULL,
 PRIMARY KEY(source_species,target_species),
 FOREIGN KEY(source_species) REFERENCES digimon_species(id),
 FOREIGN KEY(target_species) REFERENCES digimon_species(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS digimon_trials (
 companion_id INT UNSIGNED NOT NULL, target_species INT UNSIGNED NOT NULL,
 victories SMALLINT UNSIGNED NOT NULL DEFAULT 0, completed TINYINT UNSIGNED NOT NULL DEFAULT 0,
 catalysts SMALLINT UNSIGNED NOT NULL DEFAULT 0,
 PRIMARY KEY(companion_id,target_species),
 FOREIGN KEY(companion_id) REFERENCES digimon_collection(id) ON DELETE CASCADE,
 FOREIGN KEY(target_species) REFERENCES digimon_species(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS digimon_affinities (
 companion_id INT UNSIGNED NOT NULL, affinity VARCHAR(24) NOT NULL,
 points SMALLINT UNSIGNED NOT NULL DEFAULT 0,
 PRIMARY KEY(companion_id,affinity),
 FOREIGN KEY(companion_id) REFERENCES digimon_collection(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS digimon_training (
 companion_id INT UNSIGNED NOT NULL PRIMARY KEY, target_species INT UNSIGNED NOT NULL,
 rewarded_at BIGINT UNSIGNED NOT NULL DEFAULT 0,
 FOREIGN KEY(companion_id) REFERENCES digimon_collection(id) ON DELETE CASCADE,
 FOREIGN KEY(target_species) REFERENCES digimon_species(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
INSERT IGNORE INTO digimon_route_requirements VALUES
(103, 105, 'ARCANO', 5, 5, 1, 'La chispa del hechicero'),
(103, 108, 'OSCURIDAD', 5, 5, 1, 'El pacto de las sombras'),
(103, 112, 'ESPIRITU', 5, 5, 1, 'Voces del otro lado'),
(103, 115, 'BRUJERIA', 5, 5, 1, 'Aprendiz de la tempestad'),
(105, 106, 'ARCANO', 20, 12, 1, 'El precio del poder'),
(106, 107, 'ARCANO', 40, 20, 1, 'Romper el limite'),
(108, 109, 'OSCURIDAD', 15, 8, 1, 'Senor de la noche'),
(109, 110, 'OSCURIDAD', 30, 12, 1, 'Hambre de caos'),
(110, 111, 'OSCURIDAD', 50, 20, 1, 'Dominio del abismo'),
(112, 113, 'ESPIRITU', 15, 8, 1, 'La ultima sentencia'),
(113, 114, 'ESPIRITU', 30, 12, 1, 'El teatro de las mascaras'),
(115, 116, 'BRUJERIA', 15, 8, 1, 'Alas de la noche'),
(116, 117, 'BRUJERIA', 30, 12, 1, 'La corona de las sombras');
UPDATE digimon_evolution_edges e JOIN digimon_species s ON s.id=e.source_species JOIN digimon_species t ON t.id=e.target_species SET e.enabled=1 WHERE s.enabled=1 AND t.enabled=1 AND e.source_species IN(103,104,105,106,108,109,110,112,113,115,116) AND e.target_species BETWEEN 103 AND 117;
INSERT IGNORE INTO digimon_schema_version(version) VALUES(11);
