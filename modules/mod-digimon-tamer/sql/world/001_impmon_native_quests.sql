-- SPDX-License-Identifier: GPL-2.0-or-later
-- Standard AzerothCore quest templates. The World Digital Guide (900100)
-- exposes these only from the matching active Digimon branch in its own menu.
INSERT INTO quest_template
(ID, QuestType, QuestLevel, MinLevel, RewardXPDifficulty, RewardChoiceItemID1,
 RewardChoiceItemQuantity1, LogTitle, LogDescription, QuestDescription,
 AreaDescription, QuestCompletionLog, RequiredNpcOrGo1, RequiredNpcOrGoCount1,
 ObjectiveText1, VerifiedBuild)
VALUES
(900501,2,25,1,3,900210,1,'La chispa del hechicero','Ayuda a Impmon a demostrar su afinidad arcana.','Derrota 40 lobos jóvenes para que Impmon reúna energía arcana. Regresa con el Guía del Mundo Digital para completar el encargo.','Bosque de Elwynn', 'Has reunido la energía necesaria para la evolución.',705,40,'Derrota lobos jóvenes: 0/40',12340),
(900502,2,70,1,3,900210,1,'El precio del poder','Ayuda a Wizardmon a dominar su siguiente forma.','Derrota a 12 Anger Guard en Terrallende y vuelve con el Guía del Mundo Digital.','Terrallende', 'El Guía del Mundo Digital espera tu informe.',16952,12,'Derrota Anger Guard: 0/12',12340),
(900503,2,85,1,3,900210,1,'Romper el límite','Acompaña a Beelzemon hasta el límite de su poder.','Derrota a 20 necrófagos descompuestos en Rasganorte y vuelve con el Guía del Mundo Digital.','Rasganorte', 'El Guía del Mundo Digital espera tu informe.',31812,20,'Derrota necrófagos descompuestos: 0/20',12340),
(900504,2,25,1,3,900211,1,'El pacto de las sombras','Ayuda a Impmon a profundizar en la afinidad de oscuridad.','Derrota 40 lobos jóvenes y vuelve con el Guía del Mundo Digital.','Bosque de Elwynn', 'El Guía del Mundo Digital espera tu informe.',705,40,'Derrota lobos jóvenes: 0/40',12340),
(900505,2,45,1,3,900211,1,'Señor de la noche','Ayuda a Devimon a demostrar su dominio de la noche.','Derrota a 8 Haldarr Trickster en Azshara y vuelve con el Guía del Mundo Digital.','Azshara', 'El Guía del Mundo Digital espera tu informe.',6126,8,'Derrota Haldarr Trickster: 0/8',12340),
(900506,2,65,1,3,900211,1,'Hambre de caos','Ayuda a Myotismon a controlar su hambre de poder.','Derrota a 12 guardianes no-muertos en Terrallende y vuelve con el Guía del Mundo Digital.','Terrallende', 'El Guía del Mundo Digital espera tu informe.',21869,12,'Derrota Unliving Guardian: 0/12',12340),
(900507,2,80,1,3,900211,1,'Dominio del abismo','Acompaña a VenomMyotismon hacia su forma definitiva.','Derrota a 20 necrófagos descompuestos en Rasganorte y vuelve con el Guía del Mundo Digital.','Rasganorte', 'El Guía del Mundo Digital espera tu informe.',31812,20,'Derrota necrófagos descompuestos: 0/20',12340),
(900508,2,25,1,3,900212,1,'Voces del otro lado','Ayuda a Impmon a escuchar las voces del otro lado.','Derrota 40 lobos jóvenes y vuelve con el Guía del Mundo Digital.','Bosque de Elwynn', 'El Guía del Mundo Digital espera tu informe.',705,40,'Derrota lobos jóvenes: 0/40',12340),
(900509,2,45,1,3,900212,1,'La última sentencia','Ayuda a Bakemon a reunir energía espiritual.','Derrota a 8 momias Atal’ai en el Pantano de las Penas y vuelve con el Guía del Mundo Digital.','Pantano de las Penas', 'El Guía del Mundo Digital espera tu informe.',5263,8,'Derrota momias Atal’ai: 0/8',12340),
(900510,2,65,1,3,900212,1,'El teatro de las máscaras','Ayuda a Phantomon a preparar su gran representación.','Derrota a 12 cuerpos vengativos en Terrallende y vuelve con el Guía del Mundo Digital.','Terrallende', 'El Guía del Mundo Digital espera tu informe.',22045,12,'Derrota Vengeful Husk: 0/12',12340),
(900511,2,25,1,3,900213,1,'Aprendiz de la tempestad','Ayuda a Impmon a aprender los secretos de la brujería.','Derrota 40 lobos jóvenes y vuelve con el Guía del Mundo Digital.','Bosque de Elwynn', 'El Guía del Mundo Digital espera tu informe.',705,40,'Derrota lobos jóvenes: 0/40',12340),
(900512,2,45,1,3,900213,1,'Alas de la noche','Ayuda a Witchmon a dominar sus alas oscuras.','Derrota a 8 Haldarr Felsworn en Azshara y vuelve con el Guía del Mundo Digital.','Azshara', 'El Guía del Mundo Digital espera tu informe.',6127,8,'Derrota Haldarr Felsworn: 0/8',12340),
(900513,2,65,1,3,900213,1,'La corona de las sombras','Ayuda a LadyDevimon a reclamar su corona.','Derrota a 12 Felblade Doomguard y vuelve con el Guía del Mundo Digital.','Terrallende', 'El Guía del Mundo Digital espera tu informe.',19853,12,'Derrota Felblade Doomguard: 0/12',12340)
ON DUPLICATE KEY UPDATE
QuestLevel=VALUES(QuestLevel), MinLevel=VALUES(MinLevel), RewardXPDifficulty=VALUES(RewardXPDifficulty),
RewardChoiceItemID1=VALUES(RewardChoiceItemID1), RewardChoiceItemQuantity1=VALUES(RewardChoiceItemQuantity1),
LogTitle=VALUES(LogTitle), LogDescription=VALUES(LogDescription), QuestDescription=VALUES(QuestDescription),
AreaDescription=VALUES(AreaDescription), QuestCompletionLog=VALUES(QuestCompletionLog),
RequiredNpcOrGo1=VALUES(RequiredNpcOrGo1), RequiredNpcOrGoCount1=VALUES(RequiredNpcOrGoCount1),
ObjectiveText1=VALUES(ObjectiveText1), VerifiedBuild=VALUES(VerifiedBuild);

INSERT IGNORE INTO creature_queststarter(id,quest) VALUES
(900100,900501),(900100,900502),(900100,900503),(900100,900504),(900100,900505),
(900100,900506),(900100,900507),(900100,900508),(900100,900509),(900100,900510),
(900100,900511),(900100,900512),(900100,900513);

INSERT IGNORE INTO creature_questender(id,quest) VALUES
(900100,900501),(900100,900502),(900100,900503),(900100,900504),(900100,900505),
(900100,900506),(900100,900507),(900100,900508),(900100,900509),(900100,900510),
(900100,900511),(900100,900512),(900100,900513);

-- Native quest completion is per Tamer; repeatable flags let each stored
-- companion complete its own branch mission on the same character.
INSERT INTO quest_template_addon(ID,SpecialFlags) VALUES
(900501,1),(900502,1),(900503,1),(900504,1),(900505,1),(900506,1),(900507,1),
(900508,1),(900509,1),(900510,1),(900511,1),(900512,1),(900513,1)
ON DUPLICATE KEY UPDATE SpecialFlags=SpecialFlags | 1;
