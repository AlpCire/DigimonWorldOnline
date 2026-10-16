-- SPDX-License-Identifier: GPL-2.0-or-later
-- A server-generated token links the debit and learned row inside one transaction.
ALTER TABLE `digimon_tamer` ADD COLUMN `purchase_token` CHAR(32) CHARACTER SET `ascii` COLLATE `ascii_bin` NULL;
INSERT IGNORE INTO `digimon_schema_version` (`version`) VALUES (4);
