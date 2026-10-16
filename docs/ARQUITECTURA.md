# WORLD DIGIMON MMO — Alpha 0.3

Laboratorio para cliente WoW 3.3.5a, compilación 12340, sobre AzerothCore
`01339ccfab9cd5d3d28546f79f38ad74d394b148`. El módulo se integra en
`modules/mod-digimon-tamer`; no modifica el código del núcleo.

## Compañero y control

La criatura invocada pertenece al jugador mediante su GUID. Su inteligencia
artificial y progresión son propias. Se usa una criatura temporal en vez de la
mascota nativa, cuya experiencia queda limitada por el nivel del dueño. Esta
decisión permite progreso independiente y evita depender de la clase cazador.
El control se realiza desde el Dispositivo Sagrado y `.digimon`; todavía no hay
una barra de mascota ni una interfaz personalizada del cliente.

Se admite un actor activo por Tamer. Una invocación repetida conserva el actor
existente. Los actores guardan GUID, no punteros entre actualizaciones. Cambiar
de mapa invalida el GUID anterior y los actores sin dueño válido se retiran.

## Integridad de datos

Las tablas `digimon_*` pertenecen al módulo y residen en la base de personajes.
Las claves foráneas vinculan dueño, colección, compañero activo y técnicas.
Una clave única por dueño impide dos iniciales. Otra identifica cada captura.
Las técnicas aprendidas y las equipadas se almacenan por separado.

Las consultas en ejecución son preparadas. Las operaciones con varias escrituras
usan transacciones y una conexión síncrona propia protegida con un bloqueo.
El catálogo y los perfiles se mantienen en memoria; no se consulta la base por
cada actualización de IA. La escritura síncrona favorece la integridad en este
laboratorio; antes de ampliar concurrencia se deben medir sus tiempos.

El instalador audita colisiones antes de escribir. Inserta identificadores
propios y no reemplaza plantillas ajenas. Las migraciones son incrementales;
MySQL confirma automáticamente las operaciones DDL, por lo que una actualización
debe realizarse con copia de seguridad y el servidor detenido.

## Daño del Tamer

El filtro de daño anula el daño del jugador y de criaturas controladas ajenas
al módulo. Se conserva el daño ambiental y el daño que un personaje se inflige
a sí mismo. Otro filtro rechaza hechizos ofensivos y nuevas invocaciones del
jugador. El compañero autorizado conserva su daño. Los objetos, efectos
especiales, clases y zonas deben ampliarse en la matriz de regresión antes de
abrir el servidor a usuarios.

## Contenido de laboratorio

Los números 900100–900103 identifican guía, entrenador, compañero y criatura
salvaje; 900104–900106, las especies futuras desactivadas; 900120, el seguidor de
ensayo. El objeto 900110 es el dispositivo. Los GUID 9000000–9000002 sitúan el
ensayo en Northshire. El salvaje se coloca en una ubicación nativa de lobo a más
de 60 metros del inicio, para proteger a los personajes nuevos. El instalador
valida las referencias 69, 197, 1128, 2031 y 107 en la base instalada. Copia filas
de referencia locales, nunca archivos del cliente. El aspecto inicial es un lobo
nativo de WoW.

La plantilla del dispositivo parte de la piedra de hogar para permitir su uso
con el cliente sin modificar; el módulo intercepta su utilización. Si el módulo
se desactiva, retire el objeto del laboratorio antes de permitir nuevas sesiones.

El repositorio no incluye cliente, mapas, DBC, modelos, texturas ni credenciales.
El material de Digimon necesita una revisión independiente de permisos.

## Entrenamiento y progreso

El hook de XP humana aplica el factor una vez, también a la XP concedida por el
flujo de misiones del núcleo. Las recompensas del compañero se calculan a partir
de su propio nivel y no pasan por ese hook. El núcleo exige una marca de
participación del jugador para recompensar muertes realizadas solo por una
invocación: se marca únicamente tras daño positivo del compañero autorizado y
se conserva el umbral normal de daño. No se añade daño ficticio del humano.

Las compras comprueban entrenador, distancia, moneda, etapa y aprendizaje previo.
Un token aleatorio generado por el servidor vincula débito y nueva técnica dentro
de una transacción. Equipar exige haber aprendido y respetar el espacio de la etapa.
Los hechizos se lanzan por las reglas del servidor, con recursos y recuperación.

Los índices de Gossip son monotónicos por sesión y no se reutilizan al reconstruir
un menú. Un clic antiguo no puede convertirse en una compra o acción diferente.
Los identificadores de colección conservan sus 32 bits completos.

## Captura y colección

Se exige criatura persistente del catálogo de captura, sin dueño ni control,
viva, normal, no protegida por misión, con poca salud, alcance y línea de visión.
Debe pertenecer al mismo dueño del botín y haber recibido daño real del actor
activo actual. La IA de captura guarda esa procedencia en memoria; al evadir la
borra. Las criaturas de misión protegidas se extraen de las plantillas al inicio.

Cada aparición tiene un token durable por GUID de spawn, mapa e instancia. Una
transacción consume la recuperación, inserta la colección y reclama esa vida.
El salvaje solo desaparece tras comprobar la fila guardada. Si el proceso termina
entre confirmación y retirada, su IA recupera la reclamación al cargar el mundo.
Una nueva vida rota el token al reaparecer. El administrador debe revisar estos
marcadores si borra o reutiliza manualmente GUID de aparición.

Cambiar de activo requiere propiedad y ausencia de combate. Guarda la salud
anterior, cambia activo y principal del equipo en una transacción y sustituye el
actor. La lista se pagina de veinte en veinte para respetar el límite de Gossip.
La salud se restaura redondeando al HP más cercano para evitar pérdidas acumuladas
al serializar porcentajes.

## Preparación futura

`digimon_team` separa posiciones preparadas de almacenamiento y actor principal.
Solo la posición cero se utiliza ahora. `digimon_unlocks` describe el plan por
nivel; mostrar una ventaja planificada no aumenta el límite operativo de uno.
`digimon_evolutions` contiene requisitos desactivados y no concede técnicas.

El comando de ensayo de administrador crea tres invocaciones pasivas con IA
distinta y caducidad de treinta segundos. No pasan la comprobación de compañero
gestionado, no pueden infligir daño del jugador y no escriben progreso. Se limita
una tanda por administrador y el ajuste se entrega desactivado. La futura IA de
apoyo necesita su propia validación de combate, formación y recompensas.
