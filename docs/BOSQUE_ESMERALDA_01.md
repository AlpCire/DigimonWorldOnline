# Primera aventura del Bosque Esmeralda

Instalada en el laboratorio el 24 de octubre de 2026, sobre el mapa abierto 1001.
El servidor cargó tras la actualización. Falta recorrerla en el cliente WoW y
confirmar alturas, aggro, botín, captura, aspecto del jefe y ritmo de misiones.

## Recorrido jugable

1. En **Villaraíz**, el Guía ofrece **Una señal en el Vivero** (901001).
2. **Patamon**, en el Vivero del Primer Brote, encadena **Fragmentos entre las
   piedras** (901002): derrotar tres Pagumon desfasados.
3. Patamon ofrece **Alas fuera de fase** (901003): derrotar dos Patamon
   desfasados y hablar con Impmon junto al Árbol de la Primera Señal.
4. **Impmon** ofrece **El guardián del Eco** (901004): vencer al Eco sombrío de
   Devimon en el Círculo del Eco y volver con Wizardmon a Villaraíz.
5. **Wizardmon** ofrece **El bosque recuerda** (901005), que se entrega al Guía.

Las misiones son nativas y dan XP al Tamer. Las recompensas incluyen comida
para el Digimon, una poción de velocidad y un Núcleo de expansión. La captura
es opcional: capturar un salvaje lo retira del mundo, pero no cuenta como una
derrota para los objetivos de estas misiones.

Patamon desfasado (900170) y Pagumon desfasado (900171) son capturables con
el Digivice al debilitarlos con el compañero activo. Sus modelos reutilizan
los display IDs locales 60001 y 60004. No se añaden modelos ni texturas al
repositorio. Las condiciones de uso y redistribución de los modelos de
terceros siguen sin acreditarse públicamente.

El jefe (900174, display ID 60008) reaparece a los cinco minutos en el mundo
abierto y **no tiene bloqueo de instancia**. Lanza Descarga de las Sombras,
avisa tres segundos antes de Miedo al bajar del 50 % de salud y obtiene
Armadura demoníaca al 25 %. Su derrota da crédito y núcleo para la misión
existente de Pagumon → Impmon. El Pagumon desfasado también da el crédito y
fragmento de esa ruta. La cadena original sigue disponible en Elwynn.

## Puntos de visita para un GM

| Lugar | Comando |
|---|---|
| Villaraíz / Guía y Wizardmon | `.go xyz 1286.667 1976.667 610.200 1001` |
| Vivero / Patamon capturable | `.go xyz 1838.667 1878.667 642.688 1001` |
| Las Tres Piedras / Pagumon capturable | `.go xyz 1673.667 1966.667 638.166 1001` |
| Árbol de la Primera Señal / Impmon | `.go xyz 1486.667 2606.667 618.200 1001` |
| Círculo del Eco / jefe | `.go xyz 2078.667 2416.667 674.200 1001` |

Los comandos colocan al jugador por encima del suelo. Las criaturas usan
alturas de apoyo. Revisar a pie que los cinco grupos están sobre el terreno,
sin invadir caminos, refugios ni la estructura ceremonial. Si alguno flota,
se hunde o agrede el espacio de conversación, ajustar su `creature.guid`
correspondiente (9000030–9000039) antes de ampliar el contenido.

## Prueba manual prioritaria

- Tomar y completar 901001–901005 en orden; comprobar XP, recompensas y
  marcadores de misión. Probar al menos una entrega sin usar comandos GM.
- Debilitar y capturar uno de cada especie; verificar que el Digiequipo o
  Digialmacén conserva forma y nivel al reconectar.
- Repetir 901002/901003 matando salvajes para confirmar que las capturas no
  incrementan por error el contador de derrotas.
- Con Pagumon activo y el prólogo 900704 completado, hacer 900801–900804
  desde los NPC del Bosque; verificar crédito de Pagumon, jefe y núcleos.
- Combatir al jefe con Tamer y Digimon: observar lanzamiento de Sombra,
  anuncio y demora de Miedo, Armadura demoníaca, botín y reaparición.
- Confirmar que el cliente no crashea al cargar el display ID 60008 en el
  Círculo del Eco. El arranque del servidor y la base de datos no sustituyen
  esta prueba visual.

## Mazmorra posterior

El Bosque Esmeralda (1001) es un mapa abierto. La futura mazmorra deberá tener
**otro ID de mapa** y su propio paquete de cliente, terreno, colisiones,
entrada, `instance_template` y script de instancia. Se copiará la lógica del
encuentro, no su spawn público. El script guardará cada jefe derrotado y
restaurará su estado después de reinicios.

Recomendación inicial: normal de cinco jugadores repetible; heroica con
reinicio global diario; una futura versión de banda con reinicio semanal.
El periodo se define por mapa y dificultad en los datos de instancia del
cliente y lo gestiona `InstanceSaveMgr` del servidor. Habrá que probar
agrupamiento, reconexión, reinicio y reparto de recompensas antes de activar
un bloqueo real. No se ha creado todavía ese segundo mapa ni el bloqueo.

## Fuentes y recuperación

- `006_emerald_first_adventure.sql`: mundo y misiones.
- `026_emerald_capture.sql`: especies capturables.
- `install.py`: copia del instalador con auditoría de colisiones de estos IDs.
- `backup_pagumon_quest_text.sql`: copia de las cuatro misiones existentes
  cuyos textos se han ampliado para incluir el bosque.
- `backup_capture_mapping.tsv`: valores previos de captura de Patamon y
  Pagumon. Las filas de colección de jugadores no se han modificado.

Las copias de trabajo están instaladas en
`G:\World of Digimon\09_desarrollo\DigimonWorldOnline`.
