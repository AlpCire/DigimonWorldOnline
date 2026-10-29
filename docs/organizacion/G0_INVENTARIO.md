# G0 — Inventario inicial basado en GitHub

**Fecha de revisión:** 2026-10-25. **Estado:** PARCIAL. **Método:** inspección remota de metadatos, directorios, README y PR. NO se ha compilado, probado ni ejecutado software en un entorno de juego.

| Subsistema | Evidencia comprobada | Estado actual | Acción necesaria | Agente |
|---|---|---|---|---|
| Repositorio troncal MMO | `AlpCire/DigimonWorldOnline` rama `main`: README de laboratorio AzerothCore/3.3.5a | Estructura principal mínima | Decidir estrategia de integración después de revisar PR #1 | 04 |
| Módulo Tamer | PR #1, rama `feature/tamer-alpha-0.3`; directorios `modules/mod-digimon-tamer`, `client/WorldDigimon`, `tests/`, `docs/`, `scripts/` | CÓDIGO ENCONTRADO, PR abierto. La descripción **declara** pruebas previas, no reproducidas aquí | Revisión de código, migraciones, scripts, tests, integración cliente y regresión | 04,05,07,08,22 |
| Dependencia servidor | Descripción de PR #1 indica AzerothCore en commit `01339ccfab9cd5d3d28546f79f38ad74d394b148` | REFERENCIA DOCUMENTADA; core completo y despliegue no inspeccionados | Validar versión, submódulo, build limpio y compatibilidad | 04,12 |
| Evoluciones | PR #1 documenta evolución por especie/nivel, ejemplo Patamon→Angemon inactivo sin modelos | PARCIAL DOCUMENTADO; ramas/catalizadores/reversión quedan pendientes según PR | Prueba de persistencia y especificación de árbol mínimo | 05,13,22 |
| Web | `AlpCire/DigimonWorldOnlineWeb`: React, TS, TanStack Start, worker, schema y migraciones | CÓDIGO ENCONTRADO; README describe pruebas y backend, sin ejecución independiente en esta auditoría | Build, test, seguridad, integración real, revisar ramas y deployment | 10,12,22 |
| Web — contenidos | `docs/` contiene seguridad, avatares, foro, cuentas, chatbot, recompensas y otras especificaciones | DOCUMENTACIÓN ENCONTRADA; no equivale a features probadas | Matriz existente vs probado y compatibilidad con juego | 10,03 |
| Demo web digievolución | `AlpCire/scroll-digi-evolution`: app web con Lovable | CÓDIGO ENCONTRADO; no es prueba de digievolución del servidor | Evaluar como referencia de UI, no como sistema canónico | 20,10 |
| Launcher v0.2.1 | [Repositorio privado conectado](https://github.com/AlpCire/DigimonWorldOnline-Launcher), README y árbol Tauri 2 inspeccionados el 29/10/2026 | V1: código y documentación localizados; README declara CI, no se ejecutó el launcher aquí | Revisar manifest, tests, artefacto CI y prueba local de lanzamiento en equipo autorizado | 11,12,22 |
| DigiModelConverter | Referido en conversaciones, no localizado entre estos tres repos | NO VERIFICADO | Localizar fuente, formatos, licencias, casos de importación | 09,17,18 |
| Terrenos y mapas nuevos | Conceptos tratados; no hay revisión de archivos editables o formatos | NO VERIFICADO | Recopilar mapas y probar carga en cliente de test | 15,16,08 |
| Cartas coleccionables | Sistema descrito en conversaciones, no auditado contra datos/código | NO VERIFICADO | Localizar CSV/scripts e identificar límites de catálogo | 13,21,07 |
| Derechos de software/assets | PR señala que no distribuye cliente, modelos ni texturas, permisos no acreditados | RIESGO CRÍTICO SIN RESOLVER | Inventario de procedencia y consulta jurídica antes de distribución/monetización | 01,02,12 |

## Fuentes comprobables
- https://github.com/AlpCire/DigimonWorldOnline
- https://github.com/AlpCire/DigimonWorldOnline/pull/1
- https://github.com/AlpCire/DigimonWorldOnline/tree/feature/tamer-alpha-0.3
- https://github.com/AlpCire/DigimonWorldOnlineWeb
- https://github.com/AlpCire/scroll-digi-evolution
- https://github.com/AlpCire/DigimonWorldOnline-Launcher

## Niveles de validación
- V0: no localizado.
- V1: artefacto/código localizado, sin ejecución.
- V2: comandos de build/test ejecutados en entorno reproducible, con logs.
- V3: integración observada en juego y regresión representativa.
- V4: candidato de beta con QA, seguridad y cumplimiento revisados.

En este informe solo se acredita V1 para los sistemas con código identificado. Un README o PR puede **declarar** V2/V3 histórico pero no autoriza a convertirlo en resultado independiente de esta auditoría.

## Hallazgo operativo
La rama `feature/tamer-alpha-0.3` y su PR #1 contienen material técnico que desaconseja una reescritura precipitada. Proteger el trabajo existente y auditar por módulos antes de decidir.
