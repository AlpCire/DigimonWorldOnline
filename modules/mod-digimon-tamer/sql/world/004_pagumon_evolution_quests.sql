-- SPDX-License-Identifier: GPL-2.0-or-later
-- Native Pagumon -> Impmon quest chain. Each quest is tied to the same active
-- Pagumon in the module; the final turn-in unlocks the route and awards the
-- route catalyst plus ordinary AzerothCore quest XP to the Tamer.

INSERT INTO quest_template
(ID,QuestType,QuestLevel,MinLevel,RewardXPDifficulty,RewardItem1,RewardAmount1,
 RewardChoiceItemID1,RewardChoiceItemQuantity1,ItemDrop1,ItemDropQuantity1,
 LogTitle,LogDescription,QuestDescription,AreaDescription,QuestCompletionLog,
 RequiredNpcOrGo1,RequiredNpcOrGoCount1,RequiredNpcOrGo2,RequiredNpcOrGoCount2,
 RequiredItemId1,RequiredItemCount1,ObjectiveText1,ObjectiveText2,VerifiedBuild)
VALUES
(900801,2,1,1,2,0,0,0,0,0,0,
 'Una señal propia','Habla con Patamon, al norte de la Abadía de Villanorte.',
 'El Dispositivo Sagrado ha reconocido a Pagumon, pero sus datos de origen están incompletos. Patamon conoce una señal antigua que puede ayudaros a descubrir de dónde viene.',
 'Villanorte','Patamon ha identificado la señal de Pagumon.',900160,1,0,0,0,0,
 'Habla con Patamon',NULL,12340),
(900802,2,2,1,2,0,0,0,0,900220,3,
 'Datos dispersos','Recupera tres fragmentos de los Pagumon desincronizados y entrégaselos a Patamon.',
 'Los Pagumon desincronizados del bosque desprenden fragmentos que encajan con la señal de tu compañero. Recupera tres de sus núcleos de datos.',
 'Bosque de Elwynn','Patamon ha reconstruido parte del patrón de Pagumon.',900150,3,0,0,900220,3,
 'Derrota Pagumon desincronizados: 0/3','Recupera fragmentos de datos: 0/3',12340),
(900803,2,3,1,3,0,0,0,0,900221,1,
 'La grieta nodal','Derrota al Pagumon nodal y lleva su núcleo a Wizardmon.',
 'Impmon detectó una distorsión cerca de Villadorada. El Pagumon nodal que la alimenta guarda el dato que falta. Wizardmon puede estabilizarlo.',
 'Bosque de Elwynn','Wizardmon ha estabilizado el núcleo nodal.',900151,1,0,0,900221,1,
 'Derrota al Pagumon nodal: 0/1','Recupera el núcleo corrupto: 0/1',12340),
(900804,2,4,1,3,0,0,900210,1,0,0,
 'Un vínculo elegido','Lleva el análisis de Wizardmon al Guía del Mundo Digital en Villanorte.',
 'El patrón demuestra que Pagumon puede abrir una ruta propia hacia Impmon. Lleva el análisis al Guía. Si el vínculo está preparado, el Dispositivo guardará un catalizador arcano para la digievolución.',
 'Villanorte','El Guía ha registrado la ruta evolutiva de Pagumon.',900100,1,0,0,0,0,
 'Habla con el Guía del Mundo Digital',NULL,12340)
ON DUPLICATE KEY UPDATE
 QuestType=VALUES(QuestType),QuestLevel=VALUES(QuestLevel),MinLevel=VALUES(MinLevel),
 RewardXPDifficulty=VALUES(RewardXPDifficulty),RewardItem1=VALUES(RewardItem1),RewardAmount1=VALUES(RewardAmount1),
 RewardChoiceItemID1=VALUES(RewardChoiceItemID1),RewardChoiceItemQuantity1=VALUES(RewardChoiceItemQuantity1),
 ItemDrop1=VALUES(ItemDrop1),ItemDropQuantity1=VALUES(ItemDropQuantity1),LogTitle=VALUES(LogTitle),
 LogDescription=VALUES(LogDescription),QuestDescription=VALUES(QuestDescription),AreaDescription=VALUES(AreaDescription),
 QuestCompletionLog=VALUES(QuestCompletionLog),RequiredNpcOrGo1=VALUES(RequiredNpcOrGo1),
 RequiredNpcOrGoCount1=VALUES(RequiredNpcOrGoCount1),RequiredNpcOrGo2=VALUES(RequiredNpcOrGo2),
 RequiredNpcOrGoCount2=VALUES(RequiredNpcOrGoCount2),RequiredItemId1=VALUES(RequiredItemId1),
 RequiredItemCount1=VALUES(RequiredItemCount1),ObjectiveText1=VALUES(ObjectiveText1),
 ObjectiveText2=VALUES(ObjectiveText2),VerifiedBuild=VALUES(VerifiedBuild);

INSERT INTO quest_template_addon(ID,PrevQuestID,SpecialFlags) VALUES
(900801,900704,0),(900802,900801,0),(900803,900802,0),(900804,900803,0)
ON DUPLICATE KEY UPDATE PrevQuestID=VALUES(PrevQuestID),SpecialFlags=VALUES(SpecialFlags);

INSERT IGNORE INTO creature_queststarter(id,quest) VALUES
(900100,900801),(900160,900802),(900161,900803),(900162,900804);
INSERT IGNORE INTO creature_questender(id,quest) VALUES
(900160,900801),(900160,900802),(900162,900803),(900100,900804);
