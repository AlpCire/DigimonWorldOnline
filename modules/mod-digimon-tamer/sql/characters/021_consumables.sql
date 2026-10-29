-- Persistent care values, replaceable temporary companion effects, and crash-safe item consumption.
ALTER TABLE digimon_collection
    ADD COLUMN happiness TINYINT UNSIGNED NOT NULL DEFAULT 50 AFTER health_pct,
    ADD COLUMN damage_buff_pct TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER happiness,
    ADD COLUMN damage_buff_expires BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER damage_buff_pct,
    ADD COLUMN protection_buff_pct TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER damage_buff_expires,
    ADD COLUMN protection_buff_expires BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER protection_buff_pct;

CREATE TABLE IF NOT EXISTS digimon_consumables (
    item_entry INT UNSIGNED NOT NULL PRIMARY KEY,
    kind ENUM('FOOD','DAMAGE','PROTECTION','SPEED') NOT NULL,
    happiness_gain TINYINT UNSIGNED NOT NULL DEFAULT 0,
    magnitude TINYINT UNSIGNED NOT NULL DEFAULT 0,
    duration_seconds SMALLINT UNSIGNED NOT NULL DEFAULT 0,
    spell_id INT UNSIGNED NOT NULL DEFAULT 0,
    enabled TINYINT UNSIGNED NOT NULL DEFAULT 1,
    CONSTRAINT digimon_consumable_happiness CHECK (happiness_gain <= 100),
    CONSTRAINT digimon_consumable_magnitude CHECK (magnitude <= 50),
    CONSTRAINT digimon_consumable_duration CHECK (duration_seconds <= 3600)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO digimon_consumables(item_entry,kind,happiness_gain,magnitude,duration_seconds,spell_id,enabled) VALUES
    (900230,'FOOD',15,0,0,0,1),
    (900231,'DAMAGE',5,15,600,0,1),
    (900232,'PROTECTION',5,15,600,0,1),
    (900233,'SPEED',0,0,15,2379,1)
ON DUPLICATE KEY UPDATE kind=VALUES(kind),happiness_gain=VALUES(happiness_gain),magnitude=VALUES(magnitude),
    duration_seconds=VALUES(duration_seconds),spell_id=VALUES(spell_id),enabled=VALUES(enabled);

CREATE TABLE IF NOT EXISTS digimon_consumable_spent (
    item_guid INT UNSIGNED NOT NULL PRIMARY KEY,
    owner_guid INT UNSIGNED NOT NULL,
    item_entry INT UNSIGNED NOT NULL,
    spent_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    KEY digimon_consumable_spent_owner (owner_guid),
    CONSTRAINT digimon_consumable_spent_owner_fk FOREIGN KEY (owner_guid)
        REFERENCES digimon_tamer(owner_guid) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO digimon_schema_version(version) VALUES(21);
