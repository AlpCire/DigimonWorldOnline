-- SPDX-License-Identifier: GPL-2.0-or-later
-- User-provided Impmon family catalogue; new forms and evolution routes remain disabled until models/templates are installed.
ALTER TABLE `digimon_techniques`
    ADD COLUMN `localized_name` VARCHAR(80) NOT NULL DEFAULT '' AFTER `name`,
    ADD COLUMN `category` ENUM('OFFICIAL', 'GAME_REFERENCE', 'MMO_CUSTOM') NOT NULL DEFAULT 'GAME_REFERENCE' AFTER `special`,
    ADD COLUMN `min_level` TINYINT UNSIGNED NOT NULL DEFAULT 1 AFTER `category`,
    ADD COLUMN `cooldown_override_ms` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `min_level`,
    ADD COLUMN `damage_type` ENUM('PHYSICAL', 'FIRE', 'SHADOW', 'NATURE', 'ARCANE') NOT NULL DEFAULT 'PHYSICAL' AFTER `cooldown_override_ms`,
    ADD COLUMN `scaling_model` VARCHAR(48) NOT NULL DEFAULT 'SPELL_NATIVE_LEVEL_AND_STATS' AFTER `damage_type`,
    ADD COLUMN `effect_note` VARCHAR(255) NOT NULL DEFAULT '' AFTER `scaling_model`,
    ADD COLUMN `control_limited` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `effect_note`;

INSERT IGNORE INTO `digimon_techniques` (`id`,`name`,`spell_id`,`required_stage`,`price`,`enabled`,`special`,`localized_name`,`category`,`min_level`,`cooldown_override_ms`,`damage_type`,`scaling_model`,`effect_note`,`control_limited`) VALUES
(1000,'Poison Bubbles',4286,1,0,1,0,'Burbujas venenosas','GAME_REFERENCE',5,6000,'NATURE','SPELL_NATIVE_LEVEL_AND_STATS','Proyectiles venenosos, daño leve y veneno',0),
(1001,'Acid Bubbles',70744,1,0,1,0,'Burbujas ácidas','GAME_REFERENCE',7,10000,'NATURE','SPELL_NATIVE_LEVEL_AND_STATS','Daño de naturaleza y reducción de armadura',0),
(1002,'Tackle',1082,1,0,1,0,'Embestida','GAME_REFERENCE',9,5000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Embestida física',0),
(1003,'Night of Fire',133,1,0,1,0,'Noche de fuego','GAME_REFERENCE',10,5000,'FIRE','SPELL_NATIVE_LEVEL_AND_STATS','Proyectil de fuego oscuro',0),
(1004,'Summon',133,1,0,1,0,'Invocación','GAME_REFERENCE',13,10000,'FIRE','SPELL_NATIVE_LEVEL_AND_STATS','Invoca energía de fuego y hielo para atacar',0),
(1005,'Dash Night of Fire',100,1,0,1,0,'Embestida nocturna','GAME_REFERENCE',17,12000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Ataque de fuego durante un desplazamiento',0),
(1006,'Pillar of Fire',4629,1,0,1,0,'Pilar de fuego','GAME_REFERENCE',20,18000,'FIRE','SPELL_NATIVE_LEVEL_AND_STATS','Muro de llamas, daño periódico en área',0),
(1007,'Marca Traviesa',702,1,0,1,0,'Marca traviesa','MMO_CUSTOM',23,20000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Aumenta el daño mágico recibido por el enemigo',0),
(1008,'Thunder Cloud',403,2,0,1,0,'Nube eléctrica','GAME_REFERENCE',25,6000,'NATURE','SPELL_NATIVE_LEVEL_AND_STATS','Descarga eléctrica',0),
(1009,'Magic Game',5782,2,0,1,0,'Juego mágico','GAME_REFERENCE',29,10000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Proyectiles mágicos con posibilidad de confusión',1),
(1010,'Terror Illusion',5782,2,0,1,0,'Ilusión aterradora','GAME_REFERENCE',34,22000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Miedo o desorientación breve',1),
(1011,'Arcane Barrier',17,2,0,1,0,'Barrera arcana','MMO_CUSTOM',40,35000,'ARCANE','SPELL_NATIVE_LEVEL_AND_STATS','Escudo de absorción',0),
(1012,'Thunderstorm',421,2,0,1,0,'Tormenta eléctrica','MMO_CUSTOM',50,25000,'NATURE','SPELL_NATIVE_LEVEL_AND_STATS','Tormenta eléctrica de área',0),
(1013,'Forbidden Magic',1459,2,0,1,0,'Magia prohibida','MMO_CUSTOM',60,60000,'ARCANE','SPELL_NATIVE_LEVEL_AND_STATS','Potenciador temporal de daño mágico',0),
(1014,'Abyssal Ritual',686,2,0,1,0,'Ritual abisal','MMO_CUSTOM',68,45000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Gran ataque oscuro canalizado',0),
(1015,'Double Impact',1082,4,0,1,0,'Impacto doble','GAME_REFERENCE',70,6000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Ráfaga doble con Berenjena',0),
(1016,'Darkness Claw',1082,4,0,1,0,'Garra de oscuridad','GAME_REFERENCE',72,8000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Ataque físico oscuro con hemorragia',0),
(1017,'Quick Shot',686,4,0,1,0,'Disparo rápido','GAME_REFERENCE',75,4000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Disparos rápidos de corto cooldown',0),
(1018,'Heartbreak Shot',686,4,0,1,0,'Disparo desgarrador','GAME_REFERENCE',78,16000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Disparo de precisión y alto daño',0),
(1019,'Behemoth Charge',100,4,0,1,0,'Carga Behemoth','MMO_CUSTOM',80,25000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Carga montada, si existe soporte visual',0),
(1020,'Demon Lord''s Fury',1459,4,0,1,0,'Furia del señor demonio','MMO_CUSTOM',83,60000,'ARCANE','SPELL_NATIVE_LEVEL_AND_STATS','Aumento temporal de crítico y velocidad',0),
(1021,'Death Slinger',686,4,0,1,0,'Disparo mortal','GAME_REFERENCE',85,12000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Disparo devastador del cañón',0),
(1022,'Chaos Flare',100,4,0,1,0,'Estallido del caos','GAME_REFERENCE',87,25000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Descarga oscura de gran potencia',0),
(1023,'Double Impact: Blast',686,4,0,1,0,'Impacto doble: ráfaga','GAME_REFERENCE',89,8000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Ráfaga combinada',0),
(1024,'Juouken',5143,4,0,1,0,'Golpe del león espiritual','GAME_REFERENCE',90,30000,'ARCANE','SPELL_NATIVE_LEVEL_AND_STATS','Proyectil espiritual con forma de león',0),
(1025,'Final Annihilation',686,4,0,1,0,'Aniquilación final','MMO_CUSTOM',90,90000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Habilidad definitiva de daño en área',0),
(1026,'Death Claw',1082,2,0,1,0,'Garra mortal','GAME_REFERENCE',25,6000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Daño físico oscuro',0),
(1027,'Leather Wing',100,2,0,1,0,'Alas de cuero','GAME_REFERENCE',29,10000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Descargas desde las alas, daño en área frontal',0),
(1028,'Hell Contract',702,2,0,1,0,'Contrato infernal','GAME_REFERENCE',34,20000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Maldición que reduce estadísticas',0),
(1029,'Deadly Nail',1082,2,0,1,0,'Uña mortal','GAME_REFERENCE',40,8000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Zarpazos sucesivos',0),
(1030,'Night Raid',686,3,0,1,0,'Asalto nocturno','GAME_REFERENCE',45,10000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Ataque de murciélagos en área',0),
(1031,'Bloody Stream',686,3,0,1,0,'Corriente sangrienta','GAME_REFERENCE',49,7000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Latigazo de energía roja',0),
(1032,'Dead Scream',686,3,0,1,0,'Grito mortal','GAME_REFERENCE',53,25000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Paralización breve',1),
(1033,'Nightmare Wave',5782,3,0,1,0,'Ola de pesadilla','GAME_REFERENCE',58,30000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Confusión o sueño',1),
(1034,'Vampiric Drain',689,3,0,1,0,'Drenaje vampírico','MMO_CUSTOM',62,15000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Daño y robo de vida',0),
(1035,'Venom Infuse',4286,4,0,1,0,'Infusión de veneno','GAME_REFERENCE',65,10000,'NATURE','SPELL_NATIVE_LEVEL_AND_STATS','Daño digital corrupto y veneno',0),
(1036,'Chaos Flame',133,4,0,1,0,'Llama del caos','GAME_REFERENCE',69,14000,'FIRE','SPELL_NATIVE_LEVEL_AND_STATS','Aliento de fuego oscuro',0),
(1037,'Tyrant Savage',1082,4,0,1,0,'Salvajismo tirano','GAME_REFERENCE',73,8000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Golpe físico de gran potencia',0),
(1038,'Inferno',133,4,0,1,0,'Infierno','GAME_REFERENCE',78,20000,'FIRE','SPELL_NATIVE_LEVEL_AND_STATS','Ataque frontal de fuego',0),
(1039,'Pandemonium Flame',4629,4,0,1,0,'Llama del pandemónium','GAME_REFERENCE',80,20000,'FIRE','SPELL_NATIVE_LEVEL_AND_STATS','Llamas infernales en área',0),
(1040,'Melting Blood',686,4,0,1,0,'Sangre fundida','GAME_REFERENCE',83,12000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Ataque oscuro que aplica daño periódico',0),
(1041,'Darkness Dominion',702,4,0,1,0,'Dominio de oscuridad','MMO_CUSTOM',86,40000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Aura de debilitamiento en área',0),
(1042,'Apocalypse Flame',133,4,0,1,0,'Llama del apocalipsis','MMO_CUSTOM',90,90000,'FIRE','SPELL_NATIVE_LEVEL_AND_STATS','Técnica definitiva de fuego oscuro',0),
(1043,'Hell''s Hand',686,2,0,1,0,'Mano infernal','GAME_REFERENCE',25,6000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Golpe de mano espectral',0),
(1044,'Death Charm',689,2,0,1,0,'Talismán mortal','GAME_REFERENCE',29,12000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Maldición y drenaje de vida',0),
(1045,'Nightmare Wave',5782,2,0,1,0,'Ola de pesadilla','GAME_REFERENCE',34,20000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Desorientación',1),
(1046,'Poison Breath',4286,2,0,1,0,'Aliento venenoso','GAME_REFERENCE',40,15000,'NATURE','SPELL_NATIVE_LEVEL_AND_STATS','Nube venenosa',0),
(1047,'Soul Chopper',1082,3,0,1,0,'Segador de almas','GAME_REFERENCE',45,7000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Golpe de guadaña',0),
(1048,'Death Sentence',702,3,0,1,0,'Sentencia de muerte','GAME_REFERENCE',50,25000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Maldición con daño retardado',0),
(1049,'Diabolic Star',686,3,0,1,0,'Estrella diabólica','GAME_REFERENCE',55,10000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Ataque oscuro con su arma encadenada',0),
(1050,'Phantom Step',100,3,0,1,0,'Paso fantasma','MMO_CUSTOM',60,20000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Desplazamiento espectral',0),
(1051,'Trump Sword',1082,4,0,1,0,'Espada Trump','GAME_REFERENCE',65,8000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Lanzamiento de espadas mágicas',0),
(1052,'Clown Trick',5782,4,0,1,0,'Truco de payaso','GAME_REFERENCE',69,20000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Desorientación y daño mágico',1),
(1053,'Masks Square',100,4,0,1,0,'Cuadrado de máscaras','GAME_REFERENCE',73,25000,'PHYSICAL','SPELL_NATIVE_LEVEL_AND_STATS','Teletransporte corto y mejora de evasión',0),
(1054,'Toy Wonderness',5143,4,0,1,0,'Maravilla de juguete','GAME_REFERENCE',78,15000,'ARCANE','SPELL_NATIVE_LEVEL_AND_STATS','Ráfaga de viento mágico',0),
(1055,'Theater of Chaos',702,4,0,1,0,'Teatro del caos','MMO_CUSTOM',80,60000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Área de ilusiones y debilitamiento',0),
(1056,'Aquary Pressure',686,2,0,1,0,'Presión acuática','GAME_REFERENCE',25,7000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Proyectil de agua a presión',0),
(1057,'Baluluna Gale',5143,2,0,1,0,'Vendaval lunar','GAME_REFERENCE',29,9000,'ARCANE','SPELL_NATIVE_LEVEL_AND_STATS','Ataque mágico de viento',0),
(1058,'Poison Storm',4286,2,0,1,0,'Tormenta venenosa','GAME_REFERENCE',34,18000,'NATURE','SPELL_NATIVE_LEVEL_AND_STATS','Área venenosa',0),
(1059,'Witch Barrier',17,2,0,1,0,'Barrera de bruja','MMO_CUSTOM',40,30000,'ARCANE','SPELL_NATIVE_LEVEL_AND_STATS','Escudo mágico',0),
(1060,'Darkness Wave',686,3,0,1,0,'Ola de oscuridad','GAME_REFERENCE',45,10000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Oleada de energía oscura en forma de murciélagos',0),
(1061,'Darkness Spear',686,3,0,1,0,'Lanza de oscuridad','GAME_REFERENCE',49,8000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Ataque perforante',0),
(1062,'Poison',4286,3,0,1,0,'Veneno','GAME_REFERENCE',53,14000,'NATURE','SPELL_NATIVE_LEVEL_AND_STATS','Energía corrupta con daño periódico',0),
(1063,'Dark Seduction',5782,3,0,1,0,'Seducción oscura','MMO_CUSTOM',58,25000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Debilita temporalmente al enemigo',0),
(1064,'Nightmare Wings',686,3,0,1,0,'Alas de pesadilla','MMO_CUSTOM',62,20000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Ataque oscuro de área',0),
(1065,'Phantom Pain',686,4,0,1,0,'Dolor fantasma','GAME_REFERENCE',65,12000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Ataque oscuro de gran potencia',0),
(1066,'Nazar Nail',4286,4,0,1,0,'Uña Nazar','GAME_REFERENCE',69,8000,'NATURE','SPELL_NATIVE_LEVEL_AND_STATS','Zarpazo venenoso',0),
(1067,'Dark Temptation',5782,4,0,1,0,'Tentación oscura','MMO_CUSTOM',73,30000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Control breve de un enemigo',1),
(1068,'Queen of Darkness',1459,4,0,1,0,'Reina de la oscuridad','MMO_CUSTOM',78,45000,'ARCANE','SPELL_NATIVE_LEVEL_AND_STATS','Potenciador de magia oscura',0),
(1069,'Seven Sins',702,4,0,1,0,'Siete pecados','MMO_CUSTOM',80,90000,'SHADOW','SPELL_NATIVE_LEVEL_AND_STATS','Técnica definitiva de maldición',0);

INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (104,900133,'Pagumon',0,1,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Pagumon',104);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (105,900134,'Wizardmon',0,2,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Wizardmon',105);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (106,900135,'Beelzemon',0,4,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Beelzemon',106);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (107,900136,'Beelzemon Blast Mode',0,4,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Beelzemon Blast Mode',107);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (108,900137,'Devimon',0,2,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Devimon',108);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (109,900138,'Myotismon',0,3,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Myotismon',109);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (110,900139,'VenomMyotismon',0,4,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('VenomMyotismon',110);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (111,900140,'MaloMyotismon',0,4,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('MaloMyotismon',111);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (112,900141,'Bakemon',0,2,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Bakemon',112);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (113,900142,'Phantomon',0,3,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Phantomon',113);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (114,900143,'Piedmon',0,4,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Piedmon',114);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (115,900144,'Witchmon',0,2,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Witchmon',115);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (116,900145,'LadyDevimon',0,3,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('LadyDevimon',116);
INSERT IGNORE INTO `digimon_species` (`id`,`creature_entry`,`name`,`starter_enabled`,`stage`,`enabled`) VALUES (117,900146,'Lilithmon',0,4,0);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Lilithmon',117);
INSERT IGNORE INTO `digimon_aliases` (`name`,`species_id`) VALUES ('Impmon',103);
INSERT IGNORE INTO `digimon_species_techniques` (`species_id`,`technique_id`) VALUES
(104,1000),
(104,1001),
(104,1002),
(103,1003),
(103,1004),
(103,1005),
(103,1006),
(103,1007),
(105,1008),
(105,1009),
(105,1010),
(105,1011),
(105,1012),
(105,1013),
(105,1014),
(106,1015),
(106,1016),
(106,1017),
(106,1018),
(106,1019),
(106,1020),
(107,1021),
(107,1022),
(107,1023),
(107,1024),
(107,1025),
(108,1026),
(108,1027),
(108,1028),
(108,1029),
(109,1030),
(109,1031),
(109,1032),
(109,1033),
(109,1034),
(110,1035),
(110,1036),
(110,1037),
(110,1038),
(111,1039),
(111,1040),
(111,1041),
(111,1042),
(112,1043),
(112,1044),
(112,1045),
(112,1046),
(113,1047),
(113,1048),
(113,1049),
(113,1050),
(114,1051),
(114,1052),
(114,1053),
(114,1054),
(114,1055),
(115,1056),
(115,1057),
(115,1058),
(115,1059),
(116,1060),
(116,1061),
(116,1062),
(116,1063),
(116,1064),
(117,1065),
(117,1066),
(117,1067),
(117,1068),
(117,1069);

INSERT IGNORE INTO `digimon_evolution_edges` (`source_species`,`target_species`,`required_level`,`enabled`,`classification`,`source_note`,`confidence`) VALUES
(104,103,10,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(103,105,25,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(103,108,25,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(103,112,25,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(103,115,25,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(105,106,70,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(106,107,85,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(108,109,45,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(109,110,65,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(110,111,80,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(112,113,45,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(113,114,65,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(115,116,45,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900),
(116,117,65,0,'GAME_REFERENCE','User-provided Impmon family design; disabled pending model/template validation',0.900);
INSERT IGNORE INTO `digimon_schema_version` (`version`) VALUES (10);
