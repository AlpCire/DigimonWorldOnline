-- Six active roster slots. Collection rows outside this roster occupy paid/free storage.
ALTER TABLE digimon_tamer
    ADD COLUMN storage_boxes SMALLINT UNSIGNED NOT NULL DEFAULT 2 AFTER collection_capacity;

CREATE TABLE IF NOT EXISTS digimon_party (
    owner_guid INT UNSIGNED NOT NULL,
    slot TINYINT UNSIGNED NOT NULL,
    companion_id INT UNSIGNED NOT NULL,
    PRIMARY KEY (owner_guid, slot),
    UNIQUE KEY digimon_party_companion (companion_id),
    CONSTRAINT digimon_party_owner_fk FOREIGN KEY (owner_guid)
        REFERENCES digimon_tamer(owner_guid) ON DELETE CASCADE,
    CONSTRAINT digimon_party_companion_fk FOREIGN KEY (companion_id)
        REFERENCES digimon_collection(id) ON DELETE CASCADE,
    CONSTRAINT digimon_party_slot_check CHECK (slot < 6)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Preserve the current active Digimon as the first party member, then fill up to six.
INSERT IGNORE INTO digimon_party(owner_guid,slot,companion_id)
SELECT ranked.owner_guid, ranked.party_slot, ranked.id
FROM (
    SELECT c.owner_guid,c.id,
        ROW_NUMBER() OVER (PARTITION BY c.owner_guid
            ORDER BY (a.companion_id=c.id) DESC,c.id) - 1 AS party_slot
    FROM digimon_collection c
    LEFT JOIN digimon_active a ON a.owner_guid=c.owner_guid
) ranked
WHERE ranked.party_slot < 6;

-- Existing collections remain accessible after migration; add boxes only when needed.
UPDATE digimon_tamer t
LEFT JOIN (
    SELECT c.owner_guid,COUNT(*) AS stored_count
    FROM digimon_collection c
    LEFT JOIN digimon_party p ON p.companion_id=c.id
    WHERE p.companion_id IS NULL
    GROUP BY c.owner_guid
) counts ON counts.owner_guid=t.owner_guid
SET t.storage_boxes=GREATEST(2,LEAST(100,CEIL(COALESCE(counts.stored_count,0)/15)));

INSERT IGNORE INTO digimon_schema_version(version) VALUES(22);
