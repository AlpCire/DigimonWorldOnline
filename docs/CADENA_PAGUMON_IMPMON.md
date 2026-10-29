# Cadena de Pagumon → Impmon

La cadena se activa después de la historia inicial y solo con Pagumon como compañero activo. Son misiones nativas de AzerothCore: el Tamer recibe la experiencia normal de misión y el registro guarda a qué compañero queda vinculada cada etapa. Completar la última etapa registra la misión de evolución, añade 5 puntos de Empatía y entrega el catalizador Arcano (900210). Impmon sigue requiriendo nivel 10; completar la historia no salta ese requisito.

| Misión | Inicio | Objetivo | Entrega |
| --- | --- | --- | --- |
| Una señal propia | Guía del Mundo Digital, Villanorte | Hablar con Patamon | Patamon |
| Datos dispersos | Patamon | Derrotar 3 Pagumon desincronizados y recoger 3 fragmentos | Patamon |
| La grieta nodal | Impmon, Villadorada | Derrotar el Pagumon nodal y recoger su núcleo | Wizardmon |
| Un vínculo elegido | Wizardmon | Informar al Guía de Villanorte | Guía; catalizador Arcano |

La ruta aparece en **Encargos de evolución** en el Guía. El jugador acepta el primer encargo allí; luego sigue los NPC y el registro de misiones. El objetivo de combate requiere las criaturas exactas 900150 y 900151. El servidor vincula cada misión al ID del Pagumon que estaba activo al aceptarla y valida esa identidad al entregarla.

## Prueba manual

1. Con un personaje Humano que terminó la introducción, obtiene el Dispositivo Sagrado y elige Pagumon, invoca al compañero.
2. En el Guía, abre **Encargos de evolución** y acepta **Una señal propia**. Repite con Patamon, no debería permitir iniciarla.
3. Completa los cuatro encargos siguiendo los NPC indicados. Confirma que los objetivos cuentan las criaturas exactas y que el Tamer gana XP de misión.
4. Tras la última entrega, confirma que recibes el catalizador 900210, Empatía +5 y la ruta queda registrada.
5. A nivel 9, la evolución debe seguir bloqueada; al llegar a nivel 10, debe permitir Pagumon → Impmon y conservar nivel y progreso.
6. Cambia de compañero durante una misión: la entrega no debe transferir la ruta al nuevo compañero.

Las migraciones son `world/004_pagumon_evolution_quests.sql` y `characters/020_pagumon_route_quests.sql`. No se añaden modelos ni activos de terceros.
