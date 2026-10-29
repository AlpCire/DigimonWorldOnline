-- A safe, native introduction chain for new human Tamers.
-- Four conversation quests lead to the Sacred Device; no combat or hostile spawns.
INSERT INTO quest_template
(ID,QuestType,QuestLevel,MinLevel,RewardXPDifficulty,RewardItem1,RewardAmount1,
 ItemDrop1,ItemDropQuantity1,LogTitle,LogDescription,QuestDescription,AreaDescription,
 QuestCompletionLog,RequiredNpcOrGo1,RequiredNpcOrGoCount1,RequiredNpcOrGo2,
 RequiredNpcOrGoCount2,RequiredItemId1,RequiredItemCount1,ObjectiveText1,ObjectiveText2,VerifiedBuild)
VALUES
(900701,2,1,1,2,0,0,0,0,'Despertar en el Mundo Digital',
 'Habla con la cronista junto a la Abadía de Villanorte.',
 'Una luz atravesó el cielo y apareciste en Villanorte sin recordar cómo llegaste. El Guía cree que la cronista vio la señal.',
 'Villanorte','La cronista ha reconocido la energía de un portal digital.',900163,1,0,0,0,0,
 'Habla con la cronista',NULL,12340),
(900702,2,2,1,2,0,0,0,0,'Recuerdos fragmentados',
 'Busca a la exploradora en el camino de Villadorada.',
 'La cronista encontró datos adheridos a tu equipo. Una exploradora de Elwynn estudia señales parecidas y quizá pueda reconstruir tu llegada.',
 'Bosque de Elwynn','La exploradora ha identificado una ruta entre mundos.',900164,1,0,0,0,0,
 'Habla con la exploradora',NULL,12340),
(900703,2,3,1,2,0,0,0,0,'Una voz en Villadorada',
 'Entrega el mapa de datos al mensajero de Villadorada.',
 'El mapa apunta a un umbral que se abrió durante una tormenta de datos. El mensajero conoce a alguien que puede ayudarte a cruzar ese recuerdo.',
 'Villadorada','El mensajero ha reconocido el emblema del Guía.',900165,1,0,0,0,0,
 'Habla con el mensajero',NULL,12340),
(900704,2,4,1,3,900110,1,0,0,'El primer vínculo',
 'Vuelve con el Guía de Villanorte para recibir tu Dispositivo Sagrado.',
 'Ya recuerdas lo esencial: una brecha te arrastró al Mundo Digital. El Guía puede entregarte un Dispositivo Sagrado para encontrar a tu primer compañero y proteger vuestro vínculo.',
 'Villanorte','Has recibido el Dispositivo Sagrado. Elige tu primer Digimon desde el menú del Guía.',900100,1,0,0,0,0,
 'Habla con el Guía del Mundo Digital',NULL,12340)
ON DUPLICATE KEY UPDATE
QuestType=VALUES(QuestType),QuestLevel=VALUES(QuestLevel),MinLevel=VALUES(MinLevel),
RewardXPDifficulty=VALUES(RewardXPDifficulty),RewardItem1=VALUES(RewardItem1),RewardAmount1=VALUES(RewardAmount1),
ItemDrop1=VALUES(ItemDrop1),ItemDropQuantity1=VALUES(ItemDropQuantity1),LogTitle=VALUES(LogTitle),
LogDescription=VALUES(LogDescription),QuestDescription=VALUES(QuestDescription),AreaDescription=VALUES(AreaDescription),
QuestCompletionLog=VALUES(QuestCompletionLog),RequiredNpcOrGo1=VALUES(RequiredNpcOrGo1),
RequiredNpcOrGoCount1=VALUES(RequiredNpcOrGoCount1),RequiredNpcOrGo2=VALUES(RequiredNpcOrGo2),
RequiredNpcOrGoCount2=VALUES(RequiredNpcOrGoCount2),RequiredItemId1=VALUES(RequiredItemId1),
RequiredItemCount1=VALUES(RequiredItemCount1),ObjectiveText1=VALUES(ObjectiveText1),
ObjectiveText2=VALUES(ObjectiveText2),VerifiedBuild=VALUES(VerifiedBuild);

INSERT INTO quest_template_addon(ID,PrevQuestID,SpecialFlags) VALUES
(900701,0,2),(900702,900701,0),(900703,900702,0),(900704,900703,0)
ON DUPLICATE KEY UPDATE PrevQuestID=VALUES(PrevQuestID),SpecialFlags=VALUES(SpecialFlags);

INSERT IGNORE INTO creature_queststarter(id,quest) VALUES
(900100,900701),(900163,900702),(900164,900703),(900165,900704);
INSERT IGNORE INTO creature_questender(id,quest) VALUES
(900163,900701),(900164,900702),(900165,900703),(900100,900704);
