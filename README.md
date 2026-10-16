# WORLD DIGIMON MMO — Alpha 0.3 de laboratorio

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
- Un compañero operativo; catálogo futuro de oso, felino y rapaz desactivado.
- Tablas de equipo, requisitos de evolución desactivados y desbloqueos por nivel.
- Experimento de tres seguidores temporales, reservado a administradores y desactivado por defecto.

El control utiliza Gossip estándar. El compañero es una criatura invocada con IA
propia; no depende de la mascota nativa del cazador y no tiene barra de mascota.
La Alpha permite descansar y recuperar salud gratis fuera de combate.

## Empezar

1. Leer [instalación](docs/INSTALACION.md) y trabajar sobre bases de ensayo.
2. Usar AzerothCore `01339ccfab9cd5d3d28546f79f38ad74d394b148`.
3. Compilar, aplicar el instalador con el mundo detenido y arrancar el binario propio.
4. Con una cuenta normal, crear un humano y hablar con el guía de Northshire.
5. Elegir Lobo digital, abrir el dispositivo, seleccionar un enemigo y ordenar atacar.

Consultar [arquitectura](docs/ARQUITECTURA.md), [pruebas ejecutadas](docs/PRUEBAS.md),
[protocolo dentro del cliente](docs/PRUEBAS_MANUALES.md),
[limitaciones](docs/LIMITACIONES.md) y [siguiente desarrollo](docs/PLAN_ALPHA_03.md).

## Procedencia

Código nuevo bajo GPL-2.0-or-later; pruebas con AzerothGhost MIT. No se incluyen
cliente, mapas, DBC, modelos, texturas, bases de cuentas ni contraseñas.
El lobo utiliza referencias a recursos de la instalación local de WoW.
No se ha certificado ningún modelo Digimon de terceros.
El ZIP Alpha 0.2 ya se ha inventariado y comparado:
[auditoría y diferencias](docs/AUDITORIA_ALPHA_02.md). El kit de modelos original
sigue pendiente. Véase [permisos](docs/PERMISOS_RECURSOS.md).
