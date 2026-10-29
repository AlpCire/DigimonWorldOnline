# Primera cadena de misiones humanas

La migración `002_human_story_quests.sql` incorpora una cadena nativa de cinco pasos para personajes humanos en Northshire y Villadorada. Las misiones usan el registro y las recompensas normales de AzerothCore, así que conceden experiencia al Tamer y guardan el progreso en el personaje.

1. **Señales entre los pinos** — Guía del Mundo Digital, Villanorte → habla con Patamon.
2. **Datos de Pagumon** — Patamon → recoge cuatro Fragmentos de Datos de Pagumon desincronizados en el bosque.
3. **Rumores en Villadorada** — Patamon → habla con Impmon en Villadorada.
4. **El núcleo de la distorsión** — Impmon → derrota al Pagumon nodal y entrega su núcleo a Wizardmon.
5. **Informe para el Guía** — Wizardmon → entrega el informe cifrado al Guía en Villanorte.

Los Digimon amistosos usan los modelos locales ya instalados. Se crean tres encuentros de Pagumon en torno a los puntos de lobos de Northshire y un Pagumon nodal en las afueras de Villadorada. Los enemigos reaparecen como criaturas normales; no forman parte del equipo, no se capturan y no sustituyen a los lobos existentes.

Los objetivos de conversación se acreditan al interactuar con el NPC correspondiente. El informe y los fragmentos usan iconos de cristales que ya existen en el cliente 3.3.5a. Las recompensas de experiencia se mantienen en las plantillas nativas de misión.

## Prueba manual

1. Habla con el Guía del Mundo Digital en Villanorte y acepta **Señales entre los pinos**.
2. Habla con Patamon, recoge cuatro fragmentos de los Pagumon desincronizados y entrégaselos.
3. Habla con Impmon en Villadorada y continúa hasta derrotar al Pagumon nodal.
4. Entrega el núcleo a Wizardmon y lleva el informe al Guía.
5. Comprueba el registro de misiones, la experiencia del personaje y el inventario tras cada entrega.

Las misiones usan los IDs 900601–900605; NPCs y encuentros 900150–900162; objetos 900220–900222; apariciones 9000010–9000016.
