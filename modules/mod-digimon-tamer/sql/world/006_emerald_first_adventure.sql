-- SPDX-License-Identifier: GPL-2.0-or-later
-- Bosque Esmeralda (map 1001): first small native quest chain, two capture species,
-- and a public boss prototype. No instance lockout is attached to this map.
-- Uses display IDs already present in the private local client patch; no assets
-- are included or licensed for redistribution by this SQL.

INSERT INTO creature_template
(entry,name,subname,KillCredit1,minlevel,maxlevel,faction,npcflag,`rank`,DamageModifier,
 BaseAttackTime,unit_class,type,lootid,AIName,ScriptName,HealthModifier,
 ExperienceModifier,unit_flags,VerifiedBuild)
VALUES
(900170,'Patamon desfasado','Digimon salvaje',0,4,4,14,0,0,0.70,
 2000,1,1,0,'','DigimonWildAI',3.0,1,0,0),
(900171,'Pagumon desfasado','Digimon salvaje',900150,3,3,14,0,0,0.70,
 2000,1,1,900171,'','DigimonWildAI',3.0,1,0,0),
(900174,'Eco sombrío de Devimon','Guardián del Eco',900151,12,12,14,0,1,0.85,
 2000,1,1,900174,'SmartAI','',8.0,1,0,0)
ON DUPLICATE KEY UPDATE
 name=VALUES(name),subname=VALUES(subname),KillCredit1=VALUES(KillCredit1),
 minlevel=VALUES(minlevel),maxlevel=VALUES(maxlevel),faction=VALUES(faction),
 npcflag=VALUES(npcflag),`rank`=VALUES(`rank`),DamageModifier=VALUES(DamageModifier),
 BaseAttackTime=VALUES(BaseAttackTime),unit_class=VALUES(unit_class),type=VALUES(type),
 lootid=VALUES(lootid),AIName=VALUES(AIName),ScriptName=VALUES(ScriptName),
 HealthModifier=VALUES(HealthModifier),ExperienceModifier=VALUES(ExperienceModifier),
 unit_flags=VALUES(unit_flags),VerifiedBuild=VALUES(VerifiedBuild);

INSERT INTO creature_template_model
(CreatureID,Idx,CreatureDisplayID,DisplayScale,Probability,VerifiedBuild)
VALUES
(900170,0,60001,1,1,0),
(900171,0,60004,1,1,0),
(900174,0,60008,2.8125,1,0)
ON DUPLICATE KEY UPDATE CreatureDisplayID=VALUES(CreatureDisplayID),
 DisplayScale=VALUES(DisplayScale),Probability=VALUES(Probability),VerifiedBuild=VALUES(VerifiedBuild);

-- The forest Pagumon and boss can also satisfy the existing native
-- Pagumon -> Impmon route via KillCredit1 and these same quest items.
INSERT INTO creature_loot_template
(Entry,Item,Reference,Chance,QuestRequired,LootMode,GroupId,MinCount,MaxCount,Comment)
VALUES
(900171,900220,0,100,1,1,0,1,1,'Forest Pagumon route data'),
(900174,900221,0,100,1,1,0,1,1,'Forest nodal core')
ON DUPLICATE KEY UPDATE Reference=VALUES(Reference),Chance=VALUES(Chance),
 QuestRequired=VALUES(QuestRequired),LootMode=VALUES(LootMode),GroupId=VALUES(GroupId),
 MinCount=VALUES(MinCount),MaxCount=VALUES(MaxCount),Comment=VALUES(Comment);

-- Fixed positions are on the surveyed exterior arrival plateaus from
-- DigitalWorldContinent/docs/VISITA_BOSQUE_13.md (arrival Z minus 2).
-- Forest geometry remains unchanged through installed revision 18.
DELETE FROM creature WHERE id IN (900100,900160,900161,900162,900170,900171,900174)
 AND guid BETWEEN 9000030 AND 9000039;
INSERT INTO creature
(guid,id,map,position_x,position_y,position_z,orientation,spawntimesecs,curhealth,Comment)
VALUES
(9000030,900100,1001,1286.667,1976.667,608.200,0.0,120,100,'mod-digimon-tamer: emerald-first-adventure guide'),
(9000031,900160,1001,1823.667,1878.667,640.688,1.0,120,100,'mod-digimon-tamer: emerald-first-adventure Patamon'),
(9000032,900161,1001,1486.667,2606.667,616.200,2.0,120,100,'mod-digimon-tamer: emerald-first-adventure Impmon'),
(9000033,900162,1001,1301.667,1976.667,608.200,3.0,120,100,'mod-digimon-tamer: emerald-first-adventure Wizardmon'),
(9000034,900170,1001,1838.667,1878.667,640.688,1.2,90,100,'mod-digimon-tamer: emerald-first-adventure wild Patamon'),
(9000035,900170,1001,1845.667,1882.667,640.688,2.1,90,100,'mod-digimon-tamer: emerald-first-adventure wild Patamon'),
(9000036,900171,1001,1673.667,1966.667,636.166,0.5,90,100,'mod-digimon-tamer: emerald-first-adventure wild Pagumon'),
(9000037,900171,1001,1680.667,1962.667,636.166,1.5,90,100,'mod-digimon-tamer: emerald-first-adventure wild Pagumon'),
(9000038,900171,1001,1666.667,1970.667,636.166,2.5,90,100,'mod-digimon-tamer: emerald-first-adventure wild Pagumon'),
(9000039,900174,1001,2078.667,2416.667,672.200,3.0,300,100,'mod-digimon-tamer: emerald-first-adventure public boss');

DELETE FROM smart_scripts WHERE entryorguid=900174 AND source_type=0;
INSERT INTO smart_scripts
(entryorguid,source_type,id,link,event_type,event_chance,event_flags,
 event_param1,event_param2,event_param3,event_param4,
 action_type,action_param1,action_param2,target_type,comment)
VALUES
(900174,0,0,0,4,100,0,0,0,0,0,1,0,0,1,'Devimon Echo - aggro - warn players'),
(900174,0,1,0,0,100,0,6000,8000,11000,14000,11,686,0,2,'Devimon Echo - combat - interruptible Shadow Bolt'),
(900174,0,2,0,2,100,1,0,50,0,0,1,1,3000,1,'Devimon Echo - below 50 percent - three-second Fear warning'),
(900174,0,3,0,52,100,0,1,900174,0,0,11,5782,0,2,'Devimon Echo - warning ends - Fear on current target'),
(900174,0,4,0,2,100,1,0,25,0,0,11,706,0,1,'Devimon Echo - below 25 percent - Demon Armor'),
(900174,0,5,0,6,100,0,0,0,0,0,1,2,0,1,'Devimon Echo - death - announce end of distortion');

DELETE FROM creature_text WHERE CreatureID=900174 AND GroupID IN (0,1,2);
INSERT INTO creature_text
(CreatureID,GroupID,ID,Text,Type,Language,Probability,comment)
VALUES
(900174,0,0,'¡El Eco ha despertado! Probad si podéis interrumpir mi sombra.',14,0,100,'Devimon Echo aggro'),
(900174,1,0,'¡Retroceded! El Eco va a sembrar miedo.',14,0,100,'Devimon Echo Fear warning'),
(900174,2,0,'El Eco se apaga... por ahora.',12,0,100,'Devimon Echo defeated');

INSERT INTO quest_template
(ID,QuestType,QuestLevel,MinLevel,QuestSortID,SuggestedGroupNum,
 RewardXPDifficulty,RewardItem1,RewardAmount1,LogTitle,LogDescription,
 QuestDescription,AreaDescription,QuestCompletionLog,RequiredNpcOrGo1,
 RequiredNpcOrGoCount1,ObjectiveText1,VerifiedBuild)
VALUES
(901001,2,5,1,6202,0,2,0,0,'Una señal en el Vivero',
 'Habla con Patamon en el Vivero del Primer Brote, al noreste de Villaraíz.',
 'Los brotes guardan datos que no deberían estar aquí. Patamon vigila el Vivero y sabe cuándo empezó la alteración. Busca las Tres Piedras para orientarte y continúa hacia el noreste.',
 'Bosque Esmeralda','Patamon espera tu informe en el Vivero.',0,0,NULL,12340),
(901002,2,5,1,6202,0,2,900230,1,'Fragmentos entre las piedras',
 'Derrota a tres Pagumon desfasados cerca de Las Tres Piedras y habla con Patamon.',
 'El Eco ha desordenado sus datos. El Dispositivo también puede capturar un salvaje debilitado por tu compañero; la captura es opcional y no cuenta como derrota para este encargo.',
 'Bosque Esmeralda','Vuelve al Vivero y cuenta a Patamon lo que has visto.',900171,3,
 'Derrota Pagumon desfasados: 0/3',12340),
(901003,2,6,1,6202,0,2,900231,1,'Alas fuera de fase',
 'Derrota a dos Patamon desfasados cerca del Vivero y busca a Impmon junto al Árbol de la Primera Señal.',
 'El Eco alcanza incluso a Patamon. Ayúdale a detener a los dos que han perdido el control. Si prefieres incorporarlos a tu Digiequipo, debilítalos y utiliza el Dispositivo; necesitarás otras derrotas para cerrar el encargo.',
 'Bosque Esmeralda','Impmon espera bajo el gran árbol, al noroeste del lago.',900170,2,
 'Derrota Patamon desfasados: 0/2',12340),
(901004,2,10,1,6202,2,4,900233,1,'El guardián del Eco',
 'Derrota al Eco sombrío de Devimon en el Círculo del Eco y vuelve con Wizardmon a Villaraíz.',
 'Impmon ha seguido la señal hasta el Círculo. El guardián lanza sombras que pueden interrumpirse y cambia de táctica al perder salud. Ve con otro Tamer si necesitas apoyo.',
 'Bosque Esmeralda','Wizardmon analizará los datos del guardián en Villaraíz.',900174,1,
 'Derrota al guardián del Eco: 0/1',12340),
(901005,2,10,1,6202,0,2,900240,1,'El bosque recuerda',
 'Entrega el análisis de Wizardmon al Guía del Mundo Digital en Villaraíz.',
 'La alteración se ha detenido, pero el Eco podría regresar. El Guía conservará el registro. Si viajas con Pagumon, consulta en el Digivice la ruta hacia Impmon y los encargos que faltan.',
 'Bosque Esmeralda','El Guía registrará tu primer recorrido por el Bosque Esmeralda.',0,0,NULL,12340)
ON DUPLICATE KEY UPDATE
 QuestType=VALUES(QuestType),QuestLevel=VALUES(QuestLevel),MinLevel=VALUES(MinLevel),
 QuestSortID=VALUES(QuestSortID),SuggestedGroupNum=VALUES(SuggestedGroupNum),
 RewardXPDifficulty=VALUES(RewardXPDifficulty),RewardItem1=VALUES(RewardItem1),
 RewardAmount1=VALUES(RewardAmount1),LogTitle=VALUES(LogTitle),
 LogDescription=VALUES(LogDescription),QuestDescription=VALUES(QuestDescription),
 AreaDescription=VALUES(AreaDescription),QuestCompletionLog=VALUES(QuestCompletionLog),
 RequiredNpcOrGo1=VALUES(RequiredNpcOrGo1),RequiredNpcOrGoCount1=VALUES(RequiredNpcOrGoCount1),
 ObjectiveText1=VALUES(ObjectiveText1),VerifiedBuild=VALUES(VerifiedBuild);

INSERT INTO quest_template_addon(ID,PrevQuestID,SpecialFlags)
VALUES (901001,0,0),(901002,901001,0),(901003,901002,0),
       (901004,901003,0),(901005,901004,0)
ON DUPLICATE KEY UPDATE PrevQuestID=VALUES(PrevQuestID),SpecialFlags=VALUES(SpecialFlags);

INSERT IGNORE INTO creature_queststarter(id,quest) VALUES
(900100,901001),(900160,901002),(900160,901003),(900161,901004),(900162,901005);
INSERT IGNORE INTO creature_questender(id,quest) VALUES
(900160,901001),(900160,901002),(900161,901003),(900162,901004),(900100,901005);

-- The original Pagumon story stays playable in Elwynn and now also reads
-- correctly when the same NPCs and kill-credit substitutes are used here.
UPDATE quest_template SET
 LogDescription='Habla con Patamon en Villanorte o en el Vivero del Bosque Esmeralda.',
 QuestDescription='El Dispositivo Sagrado ha reconocido a Pagumon, pero sus datos de origen están incompletos. Patamon puede ayudarte a interpretar la señal en Villanorte o en el Vivero del Bosque Esmeralda.',
 AreaDescription='Villanorte o Bosque Esmeralda'
WHERE ID=900801;
UPDATE quest_template SET
 LogDescription='Recupera tres fragmentos de Pagumon desincronizados en Elwynn o el Bosque Esmeralda y entrégaselos a Patamon.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda'
WHERE ID=900802;
UPDATE quest_template SET
 LogDescription='Derrota un foco nodal en Elwynn o al guardián del Círculo del Eco y lleva su núcleo a Wizardmon.',
 QuestDescription='Impmon detectó una distorsión. El Pagumon nodal de Elwynn y el guardián del Círculo del Eco comparten una firma. Cualquiera de los dos protege el núcleo que Wizardmon necesita estabilizar.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda'
WHERE ID=900803;
UPDATE quest_template SET
 LogDescription='Lleva el análisis de Wizardmon a un Guía del Mundo Digital.',
 AreaDescription='Villanorte o Villaraíz'
WHERE ID=900804;
