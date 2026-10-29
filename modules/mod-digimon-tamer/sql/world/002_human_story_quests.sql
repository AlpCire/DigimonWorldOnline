-- SPDX-License-Identifier: GPL-2.0-or-later
-- A five-step native quest chain for human characters in Northshire/Elwynn.
-- NPC talk steps are credited by DigimonQuestNPC / DigimonWelcome gossip.

INSERT INTO quest_template
(ID,QuestType,QuestLevel,MinLevel,RewardXPDifficulty,RewardItem1,RewardAmount1,
 ItemDrop1,ItemDropQuantity1,LogTitle,LogDescription,QuestDescription,AreaDescription,
 QuestCompletionLog,RequiredNpcOrGo1,RequiredNpcOrGoCount1,RequiredNpcOrGo2,
 RequiredNpcOrGoCount2,RequiredItemId1,RequiredItemCount1,ObjectiveText1,ObjectiveText2,VerifiedBuild)
VALUES
(900601,2,1,1,2,0,0,0,0,'Señales entre los pinos',
 'Habla con Patamon, al norte de la Abadía de Villanorte.',
 'El Guía ha detectado una señal irregular entre los árboles. Patamon la ha seguido hasta el bosque y puede ayudarte a encontrar su origen.',
 'Bosque de Elwynn','Patamon ha confirmado que la señal procede de datos dañados.',0,0,0,0,0,0,
 'Habla con Patamon',NULL,12340),
(900602,2,2,1,2,0,0,900220,4,'Datos de Pagumon',
 'Recupera cuatro fragmentos de los Pagumon desincronizados y vuelve con Patamon.',
 'Los Pagumon que merodean por el bosque están perdiendo fragmentos de datos. Recupera cuatro antes de que la corrupción alcance la Abadía.',
 'Bosque de Elwynn','Patamon ha recibido los fragmentos y puede analizarlos.',0,0,0,0,900220,4,
 'Recupera fragmentos de datos: 0/4',NULL,12340),
(900603,2,3,1,2,0,0,0,0,'Rumores en Villadorada',
 'Lleva a Impmon el análisis de Patamon.',
 'Los fragmentos contienen una firma que Impmon reconoce. Búscalo en Villadorada y escucha lo que sabe.',
 'Villadorada','Impmon ha reconocido la firma del núcleo.',0,0,0,0,0,0,
 'Habla con Impmon en Villadorada',NULL,12340),
(900604,2,4,1,3,900222,1,900221,1,'El núcleo de la distorsión',
 'Derrota al Pagumon nodal y lleva su núcleo a Wizardmon.',
 'Impmon ha localizado el origen de la señal en el límite oriental del bosque. El Pagumon nodal protege un núcleo corrupto. Wizardmon podrá descifrarlo.',
 'Villadorada y el Bosque de Elwynn','Wizardmon ha aislado la señal y preparado un informe cifrado.',900151,1,0,0,900221,1,
 'Derrota al Pagumon nodal: 0/1','Recupera el núcleo corrupto: 0/1',12340),
(900605,2,5,1,3,0,0,0,0,'Informe para el Guía',
 'Entrega el informe cifrado al Guía del Mundo Digital en Villanorte.',
 'Wizardmon ha demostrado que los fragmentos forman una ruta de datos mayor. Devuelve su informe al Guía para que marque la señal en el registro del mundo.',
 'Bosque de Elwynn','El Guía ha registrado el primer foco de corrupción digital.',0,0,0,0,900222,1,
 'Entrega el informe al Guía',NULL,12340)
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
(900601,0,2),(900602,900601,0),(900603,900602,2),(900604,900603,0),(900605,900604,2)
ON DUPLICATE KEY UPDATE PrevQuestID=VALUES(PrevQuestID),SpecialFlags=VALUES(SpecialFlags);

INSERT IGNORE INTO creature_queststarter(id,quest) VALUES
(900100,900601),(900160,900602),(900160,900603),(900161,900604),(900162,900605);
INSERT IGNORE INTO creature_questender(id,quest) VALUES
(900160,900601),(900160,900602),(900161,900603),(900162,900604),(900100,900605);

INSERT INTO creature_loot_template
(Entry,Item,Reference,Chance,QuestRequired,LootMode,GroupId,MinCount,MaxCount,Comment)
VALUES
(900150,900220,0,100,1,1,0,1,1,'mod-digimon-tamer human quest: Pagumon data'),
(900151,900221,0,100,1,1,0,1,1,'mod-digimon-tamer human quest: nodal core')
ON DUPLICATE KEY UPDATE Reference=VALUES(Reference),Chance=VALUES(Chance),
QuestRequired=VALUES(QuestRequired),LootMode=VALUES(LootMode),GroupId=VALUES(GroupId),
MinCount=VALUES(MinCount),MaxCount=VALUES(MaxCount),Comment=VALUES(Comment);
