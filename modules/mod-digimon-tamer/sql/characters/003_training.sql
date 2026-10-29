-- SPDX-License-Identifier: GPL-2.0-or-later
-- The installer applies each numbered upgrade once.
ALTER TABLE `digimon_tamer` ADD COLUMN `purchase_serial` BIGINT UNSIGNED NOT NULL DEFAULT 0;
INSERT IGNORE INTO `digimon_schema_version` (`version`) VALUES (3);
