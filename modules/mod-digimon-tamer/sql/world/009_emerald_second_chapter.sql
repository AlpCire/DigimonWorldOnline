-- SPDX-License-Identifier: GPL-2.0-or-later
-- Second native Emerald Forest chapter. Uses local models already installed;
-- no model or texture is distributed here. Deploy only after ID and terrain audit.

INSERT INTO creature_template
(entry,name,subname,minlevel,maxlevel,faction,npcflag,`rank`,DamageModifier,
 BaseAttackTime,unit_class,type,lootid,AIName,ScriptName,HealthModifier,
 ExperienceModifier,unit_flags,VerifiedBuild)
VALUES
(900172,'Impmon alterado','Digimon salvaje',6,6,14,0,0,0.70,
 2000,1,1,900172,'','DigimonWildAI',3.0,1,0,0),
(900173,'Reflejo de Wizardmon','Eco del Círculo',8,8,14,0,0,0.75,
 2000,1,1,900173,'','DigimonWildAI',3.5,1,0,0),
(900175,'Archivista de Villaraíz','Suministros digitales',10,10,35,131,0,0.5,
 2000,1,7,0,'','',1.0,0,2,0)
ON DUPLICATE KEY UPDATE
 name=VALUES(name),subname=VALUES(subname),minlevel=VALUES(minlevel),
 maxlevel=VALUES(maxlevel),faction=VALUES(faction),npcflag=VALUES(npcflag),
 `rank`=VALUES(`rank`),DamageModifier=VALUES(DamageModifier),
 BaseAttackTime=VALUES(BaseAttackTime),unit_class=VALUES(unit_class),
 type=VALUES(type),lootid=VALUES(lootid),AIName=VALUES(AIName),
 ScriptName=VALUES(ScriptName),HealthModifier=VALUES(HealthModifier),
 ExperienceModifier=VALUES(ExperienceModifier),unit_flags=VALUES(unit_flags),
 VerifiedBuild=VALUES(VerifiedBuild);

-- Take display IDs from the exact forms already used by the local install.
INSERT INTO creature_template_model
(CreatureID,Idx,CreatureDisplayID,DisplayScale,Probability,VerifiedBuild)
SELECT 900172,0,CreatureDisplayID,DisplayScale,1,0
FROM creature_template_model WHERE CreatureID=900132 AND Idx=0
ON DUPLICATE KEY UPDATE CreatureDisplayID=VALUES(CreatureDisplayID),
 DisplayScale=VALUES(DisplayScale),Probability=VALUES(Probability),
 VerifiedBuild=VALUES(VerifiedBuild);
INSERT INTO creature_template_model
(CreatureID,Idx,CreatureDisplayID,DisplayScale,Probability,VerifiedBuild)
SELECT 900173,0,CreatureDisplayID,DisplayScale,1,0
FROM creature_template_model WHERE CreatureID=900134 AND Idx=0
ON DUPLICATE KEY UPDATE CreatureDisplayID=VALUES(CreatureDisplayID),
 DisplayScale=VALUES(DisplayScale),Probability=VALUES(Probability),
 VerifiedBuild=VALUES(VerifiedBuild);
INSERT INTO creature_template_model
(CreatureID,Idx,CreatureDisplayID,DisplayScale,Probability,VerifiedBuild)
SELECT 900175,0,CreatureDisplayID,DisplayScale,1,0
FROM creature_template_model WHERE CreatureID=900100 AND Idx=0
ON DUPLICATE KEY UPDATE CreatureDisplayID=VALUES(CreatureDisplayID),
 DisplayScale=VALUES(DisplayScale),Probability=VALUES(Probability),
 VerifiedBuild=VALUES(VerifiedBuild);

INSERT INTO creature_loot_template
(Entry,Item,Reference,Chance,QuestRequired,LootMode,GroupId,MinCount,MaxCount,Comment)
VALUES
(900172,900250,0,100,1,1,0,1,1,'Emerald chapter: Impmon residual data'),
(900173,900251,0,100,1,1,0,1,1,'Emerald chapter: Wizardmon echo glyph')
ON DUPLICATE KEY UPDATE Reference=VALUES(Reference),Chance=VALUES(Chance),
 QuestRequired=VALUES(QuestRequired),LootMode=VALUES(LootMode),
 GroupId=VALUES(GroupId),MinCount=VALUES(MinCount),MaxCount=VALUES(MaxCount),
 Comment=VALUES(Comment);

-- Candidate placements on surveyed plateaus; client terrain check required.
DELETE FROM creature WHERE id IN (900172,900173,900175)
 AND guid BETWEEN 9000043 AND 9000048;
INSERT INTO creature
(guid,id,map,position_x,position_y,position_z,orientation,spawntimesecs,curhealth,Comment)
VALUES
(9000043,900175,1001,1294.667,1976.667,608.200,1.5,120,100,'mod-digimon-tamer: emerald-second-chapter archivist'),
(9000044,900172,1001,1511.667,2606.667,616.200,1.1,90,100,'mod-digimon-tamer: emerald-second-chapter Impmon'),
(9000045,900172,1001,1518.667,2610.667,616.200,2.0,90,100,'mod-digimon-tamer: emerald-second-chapter Impmon'),
(9000046,900172,1001,1525.667,2602.667,616.200,0.5,90,100,'mod-digimon-tamer: emerald-second-chapter Impmon'),
(9000047,900173,1001,2056.667,2416.667,672.200,2.5,120,100,'mod-digimon-tamer: emerald-second-chapter Wizardmon'),
(9000048,900173,1001,2063.667,2422.667,672.200,1.2,120,100,'mod-digimon-tamer: emerald-second-chapter Wizardmon');

INSERT INTO quest_template
(ID,QuestType,QuestLevel,MinLevel,QuestSortID,SuggestedGroupNum,
 RewardXPDifficulty,RewardItem1,RewardAmount1,
 RewardChoiceItemID1,RewardChoiceItemQuantity1,
 RewardChoiceItemID2,RewardChoiceItemQuantity2,
 ItemDrop1,ItemDropQuantity1,
 LogTitle,LogDescription,QuestDescription,AreaDescription,QuestCompletionLog,
 RequiredNpcOrGo1,RequiredNpcOrGoCount1,RequiredItemId1,RequiredItemCount1,
 ObjectiveText1,ObjectiveText2,VerifiedBuild)
VALUES
(901006,2,10,1,6202,0,2,0,0,0,0,0,0,0,0,
 'Otra voz bajo el árbol','Habla con Impmon junto al Árbol de la Primera Señal.',
 'El Eco ha callado, pero Impmon oye una señal que no es la suya. Averigua qué ha quedado bajo el árbol antes de abandonar el bosque.',
 'Bosque Esmeralda','Impmon ha aislado la nueva señal.',0,0,0,0,NULL,NULL,12340),
(901007,2,10,1,6202,0,3,0,0,900231,1,900232,1,900250,4,
 'Datos que no encajan','Derrota a cuatro Impmon alterados y recoge cuatro rastros de datos para Impmon.',
 'Esos reflejos repiten órdenes antiguas. Debilítalos con tu compañero y recupera sus rastros. Podrás elegir una ración ofensiva o defensiva al terminar.',
 'Bosque Esmeralda','Impmon puede seguir la señal hasta el Círculo del Eco.',900172,4,900250,4,
 'Derrota Impmon alterados: 0/4','Rastros de datos: 0/4',12340),
(901008,2,11,1,6202,0,3,900233,1,0,0,0,0,900251,2,
 'Los glifos del Círculo','Derrota a dos reflejos de Wizardmon y entrega sus glifos al Wizardmon de Villaraíz.',
 'Los reflejos custodian las últimas letras de la señal. El Círculo sigue siendo peligroso: acércate con tu compañero preparado.',
 'Bosque Esmeralda','Wizardmon ha descifrado los glifos.',900173,2,900251,2,
 'Derrota reflejos: 0/2','Glifos del Eco: 0/2',12340),
(901009,2,11,1,6202,0,2,900230,2,0,0,0,0,0,0,
 'El archivo de Villaraíz','Entrega la lectura de Wizardmon al Archivista de Villaraíz.',
 'La señal era una copia incompleta de los registros del bosque. El Archivista puede conservarla y mostrarte los suministros de cuidado disponibles.',
 'Bosque Esmeralda','El Archivista ha recibido el registro.',0,0,0,0,NULL,NULL,12340),
(901010,2,11,1,6202,0,3,900240,1,0,0,0,0,0,0,
 'Raíces de una memoria','Habla con Patamon en el Vivero para cerrar el registro.',
 'El registro pertenece a las primeras criaturas del bosque. Patamon sabrá si la distorsión ha dejado alguna huella entre los brotes.',
 'Bosque Esmeralda','Patamon ha cerrado el registro y te entrega un Núcleo de expansión.',0,0,0,0,NULL,NULL,12340)
ON DUPLICATE KEY UPDATE
 QuestType=VALUES(QuestType),QuestLevel=VALUES(QuestLevel),MinLevel=VALUES(MinLevel),
 QuestSortID=VALUES(QuestSortID),SuggestedGroupNum=VALUES(SuggestedGroupNum),
 RewardXPDifficulty=VALUES(RewardXPDifficulty),RewardItem1=VALUES(RewardItem1),
 RewardAmount1=VALUES(RewardAmount1),RewardChoiceItemID1=VALUES(RewardChoiceItemID1),
 RewardChoiceItemQuantity1=VALUES(RewardChoiceItemQuantity1),
 RewardChoiceItemID2=VALUES(RewardChoiceItemID2),
 RewardChoiceItemQuantity2=VALUES(RewardChoiceItemQuantity2),
 ItemDrop1=VALUES(ItemDrop1),ItemDropQuantity1=VALUES(ItemDropQuantity1),
 LogTitle=VALUES(LogTitle),LogDescription=VALUES(LogDescription),
 QuestDescription=VALUES(QuestDescription),AreaDescription=VALUES(AreaDescription),
 QuestCompletionLog=VALUES(QuestCompletionLog),RequiredNpcOrGo1=VALUES(RequiredNpcOrGo1),
 RequiredNpcOrGoCount1=VALUES(RequiredNpcOrGoCount1),RequiredItemId1=VALUES(RequiredItemId1),
 RequiredItemCount1=VALUES(RequiredItemCount1),ObjectiveText1=VALUES(ObjectiveText1),
 ObjectiveText2=VALUES(ObjectiveText2),VerifiedBuild=VALUES(VerifiedBuild);

INSERT INTO quest_template_addon(ID,PrevQuestID,SpecialFlags)
VALUES (901006,901005,0),(901007,901006,0),(901008,901007,0),
       (901009,901008,0),(901010,901009,0)
ON DUPLICATE KEY UPDATE PrevQuestID=VALUES(PrevQuestID),SpecialFlags=VALUES(SpecialFlags);

INSERT IGNORE INTO creature_queststarter(id,quest) VALUES
(900100,901006),(900161,901007),(900161,901008),(900162,901009),(900175,901010);
INSERT IGNORE INTO creature_questender(id,quest) VALUES
(900161,901006),(900161,901007),(900162,901008),(900175,901009),(900160,901010);

-- Existing consumables only. New quest items are not sold.
INSERT IGNORE INTO npc_vendor(entry,slot,item,maxcount,incrtime,ExtendedCost,VerifiedBuild)
VALUES (900175,0,900230,0,0,0,0),(900175,0,900231,0,0,0,0),
       (900175,0,900232,0,0,0,0),(900175,0,900233,0,0,0,0);
