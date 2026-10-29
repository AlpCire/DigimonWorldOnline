-- SPDX-License-Identifier: GPL-2.0-or-later
ALTER TABLE digimon_route_requirements ADD COLUMN catalyst_item INT UNSIGNED NOT NULL DEFAULT 0;
UPDATE digimon_route_requirements SET catalyst_item=CASE affinity WHEN 'ARCANO' THEN 900210 WHEN 'OSCURIDAD' THEN 900211 WHEN 'ESPIRITU' THEN 900212 ELSE 900213 END;
UPDATE digimon_route_requirements SET catalyst_required=0 WHERE target_species IN(105,108,112,115,109,113,116);
CREATE TABLE IF NOT EXISTS digimon_catalyst_grants (
 companion_id INT UNSIGNED NOT NULL, target_species INT UNSIGNED NOT NULL,
 item_guid INT UNSIGNED NOT NULL, PRIMARY KEY(companion_id,target_species), UNIQUE KEY(item_guid),
 FOREIGN KEY(companion_id) REFERENCES digimon_collection(id) ON DELETE CASCADE
) ENGINE=InnoDB;
CREATE TABLE IF NOT EXISTS digimon_catalyst_spent (
 item_guid INT UNSIGNED PRIMARY KEY, owner_guid INT UNSIGNED NOT NULL,
 companion_id INT UNSIGNED NOT NULL, target_species INT UNSIGNED NOT NULL,
 spent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;
INSERT IGNORE INTO digimon_schema_version(version) VALUES(13);
