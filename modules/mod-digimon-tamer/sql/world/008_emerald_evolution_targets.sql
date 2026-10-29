-- SPDX-License-Identifier: GPL-2.0-or-later
-- Move the first evolution trials to creatures already spawned on map 1001.
-- Keep the old KillCredit1 aliases on those creatures for quests accepted
-- before this migration; new objectives use the forest entries directly.

UPDATE quest_template SET
 RequiredNpcOrGo1=900171, RequiredNpcOrGoCount1=8,
 LogDescription='Reúne energía arcana derrotando a ocho Pagumon desfasados en el Bosque Esmeralda.',
 QuestDescription='La señal de Impmon reacciona a los datos dispersos por el Bosque Esmeralda. Derrota a ocho Pagumon desfasados y vuelve con el Guía de Villaraíz.',
 AreaDescription='Bosque Esmeralda',
 ObjectiveText1='Derrota Pagumon desfasados: 0/8'
WHERE ID=900501;

UPDATE quest_template SET
 RequiredNpcOrGo1=900174, RequiredNpcOrGoCount1=1,
 LogDescription='Derrota al Eco sombrío de Devimon en el Círculo del Eco.',
 QuestDescription='El Eco sombrío de Devimon concentra la oscuridad que Impmon necesita dominar. Derrótalo en el Círculo del Eco del Bosque Esmeralda y vuelve con el Guía de Villaraíz.',
 AreaDescription='Bosque Esmeralda',
 ObjectiveText1='Derrota al Eco sombrío de Devimon: 0/1'
WHERE ID=900504;

UPDATE quest_template SET
 RequiredNpcOrGo1=900171, RequiredNpcOrGoCount1=8,
 LogDescription='Escucha la señal residual de ocho Pagumon desfasados en el Bosque Esmeralda.',
 QuestDescription='Los Pagumon desfasados transmiten voces de datos antiguos. Ayuda a Impmon a descifrarlas derrotando a ocho en el Bosque Esmeralda y vuelve con el Guía de Villaraíz.',
 AreaDescription='Bosque Esmeralda',
 ObjectiveText1='Derrota Pagumon desfasados: 0/8'
WHERE ID=900508;

UPDATE quest_template SET
 RequiredNpcOrGo1=900174, RequiredNpcOrGoCount1=1,
 LogDescription='Estudia la energía del Eco sombrío de Devimon en el Bosque Esmeralda.',
 QuestDescription='Para comprender la brujería que espera a Impmon, desafía al Eco sombrío de Devimon en el Círculo del Eco. Vuelve con el Guía de Villaraíz tras derrotarlo.',
 AreaDescription='Bosque Esmeralda',
 ObjectiveText1='Derrota al Eco sombrío de Devimon: 0/1'
WHERE ID=900511;

UPDATE quest_template SET
 RequiredNpcOrGo1=900171, RequiredNpcOrGoCount1=3,
 LogDescription='Derrota a tres Pagumon desfasados y reúne cuatro fragmentos en el Bosque Esmeralda.',
 QuestDescription='Los Pagumon desfasados del Bosque Esmeralda pierden fragmentos de datos. Reúne cuatro para que Patamon los analice en el Vivero del Primer Brote.',
 AreaDescription='Bosque Esmeralda',
 ObjectiveText1='Derrota Pagumon desfasados: 0/3',
 ObjectiveText2='Recupera fragmentos de datos: 0/4'
WHERE ID=900602;

UPDATE quest_template SET
 RequiredNpcOrGo1=900174,
 LogDescription='Derrota al Eco sombrío de Devimon en el Círculo del Eco y lleva su núcleo a Wizardmon.',
 QuestDescription='El guardián del Círculo del Eco protege el núcleo con la firma que Wizardmon necesita descifrar.',
 AreaDescription='Bosque Esmeralda',
 ObjectiveText1='Derrota al Eco sombrío de Devimon: 0/1'
WHERE ID=900604;

UPDATE quest_template SET
 LogDescription='Habla con Patamon en el Vivero del Primer Brote del Bosque Esmeralda.',
 QuestDescription='El Dispositivo Sagrado ha reconocido a Pagumon, pero sus datos de origen están incompletos. Patamon puede ayudarte a interpretar su señal en el Vivero del Primer Brote.',
 AreaDescription='Bosque Esmeralda'
WHERE ID=900801;

UPDATE quest_template SET
 RequiredNpcOrGo1=900171,
 LogDescription='Derrota a tres Pagumon desfasados y reúne sus fragmentos en el Bosque Esmeralda.',
 QuestDescription='Los Pagumon desfasados del Bosque Esmeralda desprenden fragmentos que encajan con la señal de tu compañero. Recupera tres y llévaselos a Patamon.',
 AreaDescription='Bosque Esmeralda',
 ObjectiveText1='Derrota Pagumon desfasados: 0/3'
WHERE ID=900802;

UPDATE quest_template SET
 RequiredNpcOrGo1=900174,
 LogDescription='Derrota al Eco sombrío de Devimon en el Círculo del Eco y lleva su núcleo a Wizardmon.',
 QuestDescription='Impmon detectó una distorsión en el Bosque Esmeralda. El guardián del Círculo del Eco protege el dato que falta. Wizardmon puede estabilizar su núcleo.',
 AreaDescription='Bosque Esmeralda',
 ObjectiveText1='Derrota al Eco sombrío de Devimon: 0/1'
WHERE ID=900803;

UPDATE quest_template SET
 LogDescription='Lleva el análisis de Wizardmon al Guía del Mundo Digital en Villaraíz.',
 QuestDescription='El patrón demuestra que Pagumon puede abrir una ruta propia hacia Impmon. Lleva el análisis al Guía de Villaraíz para registrar el vínculo y recibir el catalizador.',
 AreaDescription='Bosque Esmeralda'
WHERE ID=900804;
