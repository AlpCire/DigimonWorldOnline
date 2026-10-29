# WORLD DIGIMON MMO: migración de la interfaz
**Estado al 29/10/2026: pausado.** La prueba de la copia aislada sin addon mostró marcos superpuestos y datos de cliente desactualizados. No cumple las condiciones de aceptación de esta página. No instalar el parche experimental en el cliente principal ni distribuirlo con el launcher. El control de juego sigue dependiendo del addon hasta que se decida otro enfoque.

## Objetivo

Que un jugador instale y abra WORLD DIGIMON MMO con un lanzador propio y vea el control del Digimon integrado en el cliente, sin instalar manualmente `WorldDigimon` en `Interface/AddOns`. El servidor seguirá siendo la fuente de verdad para captura, equipo, progresión, talentos, evoluciones y combate.

## Límites comprobados del cliente 3.3.5a

- La barra nativa de mascota ofrece tres órdenes, cuatro espacios de hechizos y tres posturas. El proyecto equipa hasta cinco técnicas, además de captura y cuidado. No caben todas en esa barra sin otra superficie de interfaz.
- La ventana nativa de talentos de mascota depende de datos DBC, familia de criatura y clase de mascota. No representa por sí sola los 27 nodos y la persistencia por Digimon ya existentes.
- La interfaz actual es Lua. Moverla a archivos `FrameXML` dentro de un MPQ podría quitar la carpeta de addon, pero sigue siendo código Lua cliente y puede requerir modificar el ejecutable 3.3.5a por su comprobación de integridad. Esto se validará en una copia privada del cliente, nunca sobre el único ejecutable.
- Los modelos y las texturas de terceros siguen en parches privados. La distribución exige acreditar permisos para cada recurso.

## Etapas y condiciones de aceptación

1. **Control nativo aislado (en curso).** Ensayo `.digimonlabbar on/off` para administradores con el compañero actual. Confirmar que aparecen Atacar, Seguir y las posturas, que responden, y que desinvocar, cambiar de mapa y reconectar limpian el control. El comando no cambia la base de datos ni el equipo.
2. **Combate completo.** Hacer que los comandos nativos pasen por las mismas comprobaciones que `.digimon`, sincronizar la postura en ambas direcciones y decidir el acceso a la quinta técnica. Validar daño, coste, enfriamiento, amenaza y sanación con Impmon, Garurumon y Angemon. No activar por defecto hasta pasar estas pruebas.
3. **Interfaz de juego.** Reorganizar Digivice, equipo de seis, cajas, talentos, evolución, XP, empatía, felicidad y minijuego de captura. Mantener una sola copia de cada dato en servidor. Evaluar en copia separada la carga por MPQ/FrameXML; si necesita tocar `Wow.exe`, documentar el cambio exacto, hash y reversión antes de ejecutarlo.
4. **Lanzador.** Verificar versión exacta de cliente, hashes, espacio, manifiesto de parches y servidor; instalar solo archivos propios y parches cuya distribución esté autorizada; conservar copias recuperables; ofrecer reparación y reversión. No incluir credenciales en el lanzador.
5. **Retirada del addon.** Solo tras validar un cliente limpio sin `WorldDigimon` en AddOns: entrada, invocación, técnicas, tres posturas, XP, talentos, rutas, capturas, almacenamiento, misiones, mapa y tickets.

## Destino de cada función

| Función actual | Autoridad | Presentación prevista |
| --- | --- | --- |
| Invocar, retirar, atacar, seguir y posturas | Servidor | Órdenes y posturas nativas si supera el ensayo |
| Cinco técnicas, costes y enfriamientos | Servidor | Cuatro espacios nativos más acceso explícito a la quinta; sin atajos que salten las validaciones |
| XP, nivel, salud, energía, empatía y felicidad | Servidor | Marco de compañero y ficha Digivice |
| Árboles de talentos y rutas evolutivas | Servidor | Ventanas del cliente adaptadas al número real de nodos y ramas |
| Equipo de seis y Digialmacén | Servidor | Fichas, cajas y vista previa del modelo |
| Captura de cinco puntos con tiempo | Servidor valida; cliente recoge clics | Ventana propia del cliente |
| Misiones, objetos y recompensas | AzerothCore | Diario, NPC y marcos nativos cuando los datos lo permitan |
| Mapa y descubrimiento | Cliente y datos de exploración | Mapa del mundo con revelado progresivo |

Las ventanas que no existen en 3.3.5a requieren código de interfaz dentro del cliente. Empaquetarlo mediante parche y lanzador elimina la instalación manual del addon; no convierte esas ventanas en funciones originales de Blizzard.

## Estado de este ensayo

- Código del módulo compilado e instalado el 25-10-2026. Solo el administrador puede ejecutar `.digimonlabbar`, y requiere `DigimonTamer.ExperimentsEnable = 1`.
- El ensayo expone órdenes y posturas; **no** coloca técnicas en la barra nativa ni sustituye la interfaz actual.
- Compilación y comprobación de estilo: correctas. Servidor reiniciado sin jugadores conectados; arranque confirmado.
- La prueba visual del cliente aislado mostró marcos superpuestos y contenido desactualizado; no se aprobó la migración. El control sin addon permanece pendiente.
- Cliente original identificado como WoW 3.3.5a, compilación 12340, SHA-256 de `Wow.exe` `BF644876709C591ACC17C0DA8CDF1814EDCC9F1E6BC109A8C0D5C38C79DC953C`.
- Copia aislada en `work/native-ui-migration/client-isolated`, con 322 archivos de Data copiados y comprobados por tamaño. No contiene `Interface/AddOns/WorldDigimon`.
- Ejecutable de prueba con la comprobación de FrameXML modificada **solo en la copia**, SHA-256 `3F783E6F9EC1D7631C70FB29AABEFA20277ABD79349751327D1C8E0DF0A5C583`. El `Wow.exe` principal conserva su hash original.
- Parche mínimo `patch-esES-b.PROBE.MPQ` empaqueta `FrameXML.toc` y una señal en el chat. Está en la copia aislada como `Data/esES/patch-esES-b.MPQ`; espera validación al entrar al juego.
- Paquete completo **experimental** `patch-esES-b.NATIVE-EXPERIMENTAL.MPQ`: 10 módulos Lua y 978 texturas, 990 entradas MPQ. Sustituye rutas de `Interface/AddOns/WorldDigimon` por `Interface/FrameXML/WorldDigimonNative`. SHA-256 `E0659E8BFC76DA6C1479A1A3CC610CEA8561B2E4DADC9D886B4DEBD898E0F7D9`. No está instalado; no es una versión aprobada para jugadores.
- Segundo candidato aislado `patch-esES-b.NATIVE-SV-EXPERIMENTAL.MPQ`, SHA-256 `E7BC8787B12DC78AE75DBF5D3C3A0874D72B898598F2F500D8CF7B7A9A0D66BA`, añade `SavedVariablesPerCharacter` a `FrameXML.toc`. Se comprobará si el cliente realmente respeta esa directiva al salir y volver a entrar. No está instalado.
- La carga de MPQ todavía no está demostrada en partida. La herramienta de control de Windows no consiguió autorización para abrir la copia de ensayo; se ha solicitado prueba manual al usuario.

## Riesgos observados del traslado directo

- `WorldDigimonDB` es una variable guardada por el sistema de addons. Al integrar los scripts directamente en FrameXML, no está confirmado que se conserve al cerrar el juego. El segundo candidato comprueba la directiva de guardado en FrameXML; si la ignora, habrá que guardar configuración y descubrimiento en servidor.
- El paquete experimental conserva la barra y ventanas existentes, solo cambia su lugar de carga. Requiere pruebas contra el cliente real y los datos actuales antes de aplicar un rediseño o activar el control nativo de combate.
- Las texturas y modelos privados no deben distribuirse desde un repositorio o lanzador público sin permisos documentados de cada recurso.

## Reversión

- Ejecutable: `/dist/bin/worldserver.pre-nativebar-20261025`.
- Configuración: `/dist/etc/modules/mod_digimon_tamer.conf.pre-nativebar-20261025`.
- Fuente exacta anterior: `DigimonTamer.pre-nativebar.cpp` en esta carpeta (SHA-256 `9F29970FA9DCDC402840196DB5B1DACB8CC9C29A116D0F42816E5DB39626A368`).
- Antes de restaurar, comprobar que no hay jugadores conectados; copiar los dos archivos respaldados a sus rutas activas y reiniciar solo `ac-worldserver`.

## Prueba inmediata

Con un Digimon vivo invocado y fuera de combate, usar `.digimonlabbar on`. Comprobar Atacar, Seguir, Agresivo, Defensivo y Pasivo. Usar `.digimonlabbar off`, retirar e invocar de nuevo y reconectar. Si el cliente se bloquea, recuperar el personaje con la barra desactivada al reiniciar y restaurar los respaldos.
