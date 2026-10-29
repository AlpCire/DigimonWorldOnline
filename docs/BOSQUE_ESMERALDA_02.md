# Bosque Esmeralda: ecos tras el guardián

Contenido preparado para **WORLD DIGIMON MMO**, mapa abierto 1001. Este
capítulo empieza después de la misión 901005, sin modificar el progreso
guardado de la primera cadena. Se ha diseñado para personajes con un solo
compañero activo y niveles aproximados 10–11.

## Recorrido nativo

| ID | Misión | Inicio → entrega | Objetivo | Recompensa |
| --- | --- | --- | --- | --- |
| 901006 | Otra voz bajo el árbol | Guía → Impmon | Hablar con Impmon | XP del Tamer |
| 901007 | Datos que no encajan | Impmon → Impmon | Derrotar 4 Impmon alterados y recoger 4 rastros | XP y elegir Ración de Filo o Estofado de Núcleo |
| 901008 | Los glifos del Círculo | Impmon → Wizardmon | Derrotar 2 reflejos de Wizardmon y recoger 2 glifos | XP y Poción de Impulso Digital |
| 901009 | El archivo de Villaraíz | Wizardmon → Archivista | Entregar la lectura | XP y 2 Galletas de Datos |
| 901010 | Raíces de una memoria | Archivista → Patamon | Hablar con Patamon | XP y Núcleo de expansión |

Los objetivos de eliminación usan criaturas concretas, sin exigir que den XP.
Los objetos de misión se obtienen con botín condicionado a misión y no tienen
precio ni uso fuera de la cadena. La captura de estos dos nuevos salvajes **no
se activa todavía**: antes hay que verificar visualmente el tamaño, la
animación y el comportamiento en el cliente. El resto de especies capturables
mantienen sus reglas.

## Encuentros, objetos y economía

| ID | Tipo | Uso |
| --- | --- | --- |
| 900172 | Impmon alterado | Salvaje nivel 6, modelo local de Impmon |
| 900173 | Reflejo de Wizardmon | Salvaje nivel 8, modelo local de Wizardmon |
| 900175 | Archivista de Villaraíz | Misiones y suministros, modelo humano existente |
| 900250 | Rastro de datos alterados | Botín de misión de Impmon alterado |
| 900251 | Glifo del Eco | Botín de misión de Reflejo de Wizardmon |

El Archivista ofrece los consumibles existentes: Galleta de Datos (3 platas),
Ración de Filo (10 platas), Estofado de Núcleo (10 platas) y Poción de Impulso
Digital (25 platas). Sus precios se fijan en `item_template.BuyPrice` solo si
el objeto conserva el precio inicial cero. El Núcleo de expansión queda como
recompensa y botín del mundo; no se vende aquí. Un futuro mercader que exija
oro más materiales digitales necesitaría un coste extendido y comprobación
de la interfaz 3.3.5a; no está simulado con un descuento ficticio.

## Puntos a verificar antes de instalar

La primera aventura todavía tenía comprobaciones de terreno pendientes.
Los nuevos grupos usan plataformas ya reconocidas, pero los desplazamientos
laterales y las alturas siguen siendo **candidatos**. Verificar en el cliente
las coordenadas, colisiones, distancia de agresión y separación del jefe.

| Lugar | GUID | Coordenada de partida |
| --- | --- | --- |
| Archivista, Villaraíz | 9000043 | 1294.667, 1976.667, 608.200 |
| Impmon alterados, Árbol | 9000044–9000046 | 1511–1525, 2602–2610, 616.200 |
| Reflejos, Círculo | 9000047–9000048 | 2056–2063, 2416–2422, 672.200 |

Comprobar también que el Archivista abre comercio y entrega/acepta misiones,
que los cuatro precios son los previstos, que la elección de recompensa se
muestra bien, y que 901010 solo aparece tras completar 901009. Un jugador con
la primera cadena terminada debe poder empezar en el Guía; un personaje nuevo
debe recorrer 901001–901005 primero. Las criaturas nuevas no deben aparecer
al iniciar sesión en medio del contacto amistoso ni bloquear el Círculo.

## Reserva de contenido posterior

Esta lista sirve para preparar capítulos, sin afirmar que estén instalados:

| Zona/nodo | Encuentro propuesto | Encargo jugable | Recompensa propuesta |
| --- | --- | --- | --- |
| Vivero del Primer Brote | Patamon y Pagumon salvajes | Reunir datos de germinación y hablar con Patamon | alimento de cuidado |
| Las Tres Piedras | Pagumon desfasados | Identificar tres marcas distintas de piedra | catalizador espiritual |
| Cabaña de la Memoria | Wizardmon | Reconstruir un registro en tres visitas | plano de objeto digital |
| Lago Espejo | reflejo de Angemon | Encuentro de sanación y protección, sin combate obligatorio | consumible defensivo |
| Paso del Guardián | Garurumon errante | Seguir huellas, derrotar intrusos concretos | objeto de velocidad limitado |
| Círculo del Eco | Eco sombrío de Devimon | Variante de grupo con mecánica de interrupción | núcleo de expansión poco frecuente |

Los nuevos capítulos deben añadir sus propios spawns, textos, botín y
condiciones antes de anunciarse como jugables. En particular, las ramas futuras
de Patamon no deben fijarse en esta fase porque el usuario prevé ampliarlas.

## Recursos y distribución

Se referencian solo IDs de modelos ya presentes en la instalación privada.
No se copian modelos, texturas ni arte de terceros. El permiso para
redistribuir estos recursos aún debe documentarse antes de empaquetar un
cliente o launcher público.
