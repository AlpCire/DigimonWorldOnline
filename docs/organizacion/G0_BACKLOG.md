# Backlog de arranque G0 — WDM-001

**Estado:** PROPUESTA NO APROBADA. **Política:** máximo 3 tareas paralelas, prioridades P0 antes de la producción de funciones nuevas.

| ID | Prio. | Responsable | Trabajo verificable | Criterio de aceptación |
|---|---|---|---|---|
| G0-01 | P0 | 04 Technical Director | Inspección de PR #1, árbol de módulos, arquitectura, compatibilidad AzerothCore | Informe con commit exacto, dependencias, riesgos, pruebas necesarias y decisión conservar/refactorizar/aislar |
| G0-02 | P0 | 22 QA Lead | Diseñar batería reproducible Tamer y client/WorldDigimon | Casos escritos para login, combate, XP, evolución, logout, restauración, permisos y errores; entorno y evidencias requeridas |
| G0-03 | P0 | 02 Product Manager | Completar inventario de repos y fuentes locales con usuario; mapear propietarios por módulo | Matriz V0–V4, pendientes reales, riesgos, enlaces comprobables y sin secretos |
| G0-04 | P0 | 01 Asesor / 12 Seguridad | Registro de licencias/derechos para core, cliente, Digimon, música, GLB, texturas y marca | Tabla de procedencia/permiso/uso previsto; bloqueo explícito de distribución dudosa |
| G0-05 | P1 | 10 Full Stack | Auditar aplicación web y su separación entre contenido demo y features operativas | `npm ci`, tests/build/TS reportados con entorno y resultado, sin desplegar |
| G0-06 | P1 | 11 Launcher | Recuperar proyecto launcher, revisar manifest, logo y actualización segura | Ruta de código, build de prueba, checksum, test de caché y recurso logo en móvil |
| G0-07 | P1 | 09 Pipeline / 18 Technical Artist | Auditar DigiModelConverter y la cadena GLB→cliente | Documento de formatos, un asset legal de prueba, hash, test de animaciones y errores |
| G0-08 | P1 | 03 Game Director / 13 Systems | Definir vertical slice jugable acotada | Especificación aprobable: zona, Tamer, compañero, combate, una evolución, 3–5 misiones y persistencia |
| G0-09 | P2 | 20 UI/UX / 21 2D | Inventariar interfaces y recursos gráficos | Matriz qué existe, qué es mockup y qué se ejecuta realmente |
| G0-10 | P2 | 24 Comunidad | Alinear comunicaciones web con estado verificado | No anunciar lanzamientos o mecánicas no probadas |

## Orden de ejecución
Lote 1: G0-01, G0-02 y G0-03. Lote 2: G0-04 y G0-05. Lote 3: G0-06 a G0-10 por dependencia. El usuario/CEO aprueba cambios de alcance, riesgo y gates.

## Gate G0 — condiciones de salida
1. Inventario de componentes con propietario y nivel de evidencia.
2. Referencias exactas a repos, commits, herramientas y entornos.
3. Tests prioritarios diseñados y al menos un intento reproducible sobre Tamer.
4. Derechos pendientes identificados y límites explícitos de distribución.
5. ADR de conservar/refactorizar/aislar/reemplazar aprobada para cada núcleo crítico.
6. Vertical slice acotada antes de autorizar ampliación masiva de features.

No hay fechas ni promesas de desarrollo hasta conocer el estado de build y dependencias.
