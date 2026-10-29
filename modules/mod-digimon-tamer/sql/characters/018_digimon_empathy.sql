-- SPDX-License-Identifier: GPL-2.0-or-later
-- Persistent, non-decaying Tamer-to-companion bond, separate from route affinities.
CREATE TABLE IF NOT EXISTS digimon_bond (
    companion_id INT UNSIGNED NOT NULL PRIMARY KEY,
    points TINYINT UNSIGNED NOT NULL DEFAULT 0,
    last_care_at BIGINT UNSIGNED NOT NULL DEFAULT 0,
    CONSTRAINT digimon_bond_points CHECK (points <= 100),
    CONSTRAINT digimon_bond_companion FOREIGN KEY (companion_id)
        REFERENCES digimon_collection(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO digimon_schema_version(version) VALUES(18);
