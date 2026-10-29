# WORLD DIGIMON MMO — estado de laboratorio

Actualización de código al 29/10/2026. El módulo, las migraciones SQL y el addon WorldDigimon 2.4.3 están en desarrollo. La prueba de interfaz integrada sin addon mostró marcos superpuestos y se ha pausado. El launcher y el mapa de Bastión Ígneo revisión 19 permanecen en el laboratorio local; este repositorio no entrega un cliente listo para jugar.

Los modelos, mapas, texturas, ejecutables y copias de seguridad privados quedan fuera del repositorio público mientras no se documente el alcance de distribución de cada recurso. La descripción de Alpha 0.4.0 que sigue es histórica.

Un Tamer dirige un Lobo digital que combate, gana experiencia propia y conserva
su colección. El humano no inflige daño. Módulo `mod-digimon-tamer` para
AzerothCore y cliente WoW **3.3.5a, build 12340**.

El servidor se ha compilado y ejecutado con cuentas de prueba conectadas por el
protocolo real del juego. La validación visual con dos clientes sigue pendiente.
Esta entrega se propone mediante una rama y una revisión; todavía no es una
certificación para abrir un servidor público.

## Incluido

- Guía de bienvenida, selección única, invocación, retirada, seguimiento y órdenes.
- Filtros de daño del Tamer y hechizos ofensivos; XP humana x0,70 una sola vez.
- XP y nivel del compañero independientes del dueño; DigiBits por derrotas válidas.
- Entrenador con compras transaccionales, técnicas aprendidas y equipadas separadas.
- Dos técnicas Rookie: Mordisco Digital y Aullido de Guerra. Etapas superiores bloqueadas.
- Dispositivo y alternativa `.digimon`, colección persistente, captura autorizada y cambio de activo.
- Modo Luchar con órdenes y técnicas en una fila propia sobre las barras inferiores mediante WorldDigimon.
- Un compañero operativo; catálogo futuro de oso, felino y rapaz desactivado.
- Evolución automática por especie y nivel; catálogo de ejemplo deshabilitado hasta instalar modelos.
- Experimento de tres seguidores temporales, reservado a administradores y desactivado por defecto.

El control de combate utiliza la barra del complemento; Gossip conserva el
entrenamiento y la colección. El compañero es una criatura invocada con IA propia;
no depende de la mascota nativa del cazador ni de su barra de mascota.
La Alpha permite descansar y recuperar salud gratis fuera de combate.

Para instalar el complemento y utilizar los controles, leer [Dispositivo y Luchar](docs/INTERFAZ_COMBATE.md).

## Empezar

1. Leer [instalación](docs/INSTALACION.md) y trabajar sobre bases de ensayo.
2. Usar AzerothCore `01339ccfab9cd5d3d28546f79f38ad74d394b148`.
3. Compilar, aplicar el instalador con el mundo detenido y arrancar el binario propio.
4. Con una cuenta normal, crear un humano y hablar con el guía de Northshire.
5. Elegir Lobo digital, abrir el dispositivo, seleccionar un enemigo y ordenar atacar.

Administración: `.listadigi`, `.spawndigi Nombre [nivel]`, `.spawndigi borrar` y
`.digilevel 22`. Catálogo CSV, técnicas por especie, selección de ramas y formas
persistentes. El Digimon llega al nivel 100. Patamon → Angemon está activo; Impmon
está preparado para prueba local con `.spawndigi Impmon`. [Guía](docs/ADMINISTRACION_CATALOGO.md).

Consultar [arquitectura](docs/ARQUITECTURA.md), [pruebas ejecutadas](docs/PRUEBAS.md),
[protocolo dentro del cliente](docs/PRUEBAS_MANUALES.md),
[limitaciones](docs/LIMITACIONES.md) y [siguiente desarrollo](docs/PLAN_ALPHA_03.md).

## Procedencia

Código nuevo bajo GPL-2.0-or-later; pruebas con AzerothGhost MIT. No se incluyen
cliente, mapas, DBC, modelos, texturas, bases de cuentas ni contraseñas.
El lobo utiliza referencias a recursos de la instalación local de WoW.
Existe una prueba gráfica privada satisfactoria de un modelo externo; no se
distribuye y no acredita compatibilidad exhaustiva ni permiso de redistribución.
El ZIP Alpha 0.2 ya se ha inventariado y comparado:
[auditoría y diferencias](docs/AUDITORIA_ALPHA_02.md). El kit de modelos original
sigue pendiente. Véase [permisos](docs/PERMISOS_RECURSOS.md).
