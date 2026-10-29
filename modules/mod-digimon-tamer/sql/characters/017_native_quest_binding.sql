-- SPDX-License-Identifier: GPL-2.0-or-later
-- Bind each accepted native quest to the exact companion that was active when
-- it was accepted, so switching companions cannot transfer its evolution credit.
CREATE TABLE IF NOT EXISTS digimon_native_quests (
    owner_guid INT UNSIGNED NOT NULL,
    quest_id INT UNSIGNED NOT NULL,
    companion_id INT UNSIGNED NOT NULL,
    target_species INT UNSIGNED NOT NULL,
    PRIMARY KEY(owner_guid, quest_id),
    KEY idx_digimon_native_quest_companion(companion_id),
    FOREIGN KEY(companion_id) REFERENCES digimon_collection(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO digimon_schema_version(version) VALUES(17);
