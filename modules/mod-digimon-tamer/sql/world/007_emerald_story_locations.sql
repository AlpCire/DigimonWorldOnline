-- SPDX-License-Identifier: GPL-2.0-or-later
-- The original Elwynn quest givers are also present in Villaraiz. Their
-- native quests must have a complete route in either location.

UPDATE quest_template SET
 LogDescription='Habla con Patamon en Elwynn o en el Vivero del Primer Brote.',
 QuestDescription='El Guía ha detectado una señal irregular entre los árboles. Patamon puede ayudarte a encontrar su origen en Elwynn o en el Bosque Esmeralda.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda'
WHERE ID=900601;
UPDATE quest_template SET
 QuestDescription='Los Pagumon desincronizados de Elwynn y del Bosque Esmeralda pierden fragmentos de datos. Recupera cuatro para que Patamon pueda analizarlos.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda'
WHERE ID=900602;
UPDATE quest_template SET
 LogTitle='La pista de Impmon',
 LogDescription='Lleva a Impmon el análisis de Patamon en Elwynn o el Bosque Esmeralda.',
 QuestDescription='Los fragmentos contienen una firma que Impmon reconoce. Búscalo en Elwynn o junto al Árbol de la Primera Señal del Bosque Esmeralda.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda',
 ObjectiveText1='Habla con Impmon'
WHERE ID=900603;
UPDATE quest_template SET
 LogDescription='Derrota al foco nodal de Elwynn o al guardián del Círculo del Eco y lleva su núcleo a Wizardmon.',
 QuestDescription='El Pagumon nodal de Elwynn y el guardián del Círculo del Eco protegen núcleos con la misma firma. Wizardmon puede descifrar cualquiera de ellos.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda',
 ObjectiveText1='Derrota un foco nodal: 0/1'
WHERE ID=900604;
UPDATE quest_template SET
 LogDescription='Entrega el informe cifrado a un Guía del Mundo Digital.',
 AreaDescription='Villanorte o Villaraíz'
WHERE ID=900605;

-- The introductory conversation chain is also offered by the forest Guide.
-- Mirror its three human speakers so it can be completed without travelling
-- back to Villadorada. Quest credit uses the same existing creature entries.
DELETE FROM creature WHERE id IN (900163,900164,900165)
 AND guid BETWEEN 9000040 AND 9000042;
INSERT INTO creature
(guid,id,map,position_x,position_y,position_z,orientation,spawntimesecs,curhealth,Comment)
VALUES
(9000040,900163,1001,1278.667,1988.667,608.200,1.0,120,100,'mod-digimon-tamer: emerald story chronicler'),
(9000041,900164,1001,1288.667,1988.667,608.200,1.0,120,100,'mod-digimon-tamer: emerald story explorer'),
(9000042,900165,1001,1298.667,1988.667,608.200,1.0,120,100,'mod-digimon-tamer: emerald story messenger');

UPDATE quest_template SET
 LogDescription='Habla con la cronista en Villanorte o Villaraíz.',
 QuestDescription='Una brecha de datos te arrastró al Mundo Digital. El Guía cree que la cronista vio la señal cuando llegaste. Puedes encontrarla en Villanorte o Villaraíz.',
 AreaDescription='Villanorte o Villaraíz'
WHERE ID=900701;
UPDATE quest_template SET
 LogDescription='Busca a la exploradora en Elwynn o Villaraíz.',
 QuestDescription='La cronista encontró datos adheridos a tu equipo. Una exploradora estudia señales parecidas y puede reconstruir tu llegada.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda'
WHERE ID=900702;
UPDATE quest_template SET
 LogTitle='Una voz entre los datos',
 LogDescription='Entrega el mapa de datos al mensajero en Elwynn o Villaraíz.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda'
WHERE ID=900703;
UPDATE quest_template SET
 LogDescription='Vuelve con el Guía en Villanorte o Villaraíz para recibir tu Dispositivo Sagrado.',
 AreaDescription='Villanorte o Villaraíz'
WHERE ID=900704;

-- Pagumon's native evolution chain shares the same forest Patamon,
-- Pagumon, boss, Wizardmon and Guide. Keep the route complete in Villaraiz.
UPDATE quest_template SET
 LogDescription='Habla con Patamon en Elwynn o en el Vivero del Primer Brote.',
 QuestDescription='El Dispositivo Sagrado ha reconocido a Pagumon, pero sus datos de origen están incompletos. Patamon conoce una señal antigua que puede ayudaros a descubrir de dónde viene.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda'
WHERE ID=900801;
UPDATE quest_template SET
 LogDescription='Recupera tres fragmentos de Pagumon desincronizados en Elwynn o en el Bosque Esmeralda y entrégaselos a Patamon.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda'
WHERE ID=900802;
UPDATE quest_template SET
 LogDescription='Derrota al foco nodal de Elwynn o al guardián del Círculo del Eco y lleva su núcleo a Wizardmon.',
 QuestDescription='Impmon detectó la distorsión en ambos bosques. El foco nodal de Elwynn y el guardián del Círculo del Eco protegen el dato que falta. Wizardmon puede estabilizar cualquiera de sus núcleos.',
 AreaDescription='Bosque de Elwynn o Bosque Esmeralda',
 ObjectiveText1='Derrota un foco nodal: 0/1'
WHERE ID=900803;
UPDATE quest_template SET
 LogDescription='Lleva el análisis de Wizardmon al Guía del Mundo Digital en Villanorte o Villaraíz.',
 AreaDescription='Villanorte o Villaraíz'
WHERE ID=900804;
