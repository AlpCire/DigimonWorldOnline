# Organización multiagente — WORLD DIGIMON MMO

**Estado:** PROPUESTA G0, pendiente de aceptación explícita. **Fecha de inicio:** 2026-10-25.

## Propósito
Coordinar un hilo principal de Codex y hasta tres subagentes especializados en paralelo. Hay 24 roles configurados para invocación bajo demanda. La actividad real ocurre durante una tarea; Git, PR y handoffs conservan las decisiones verificables entre tareas.

## Jerarquía
CEO real (usuario) → 02 Product Manager (coordinación) → especialistas; 03 Game Director controla coherencia jugable, 04 Technical Director controla contratos técnicos y 22 QA Lead define evidencia de calidad. El asesor 01 no es un segundo CEO.

## Registro de roles Codex (.codex/agents)
| ID | Rol |
|---|---|
| 01 | CEO — asesor estratégico |
| 02 | Product Manager / Executive Producer |
| 03 | Game Director |
| 04 | Technical Director / Lead Developer |
| 05 | Programador C++ — gameplay |
| 06 | Programador C++ — IA y contenido |
| 07 | Ingeniero SQL |
| 08 | Ingeniero de cliente 3.3.5a |
| 09 | Ingeniero de herramientas/pipelines |
| 10 | Programador Full Stack |
| 11 | Programador de Launcher |
| 12 | DevOps / Seguridad |
| 13 | Game Systems Designer |
| 14 | Narrative / Quest Designer |
| 15 | Level Designer |
| 16 | World Builder / Terrain Artist |
| 17 | 3D Character Artist |
| 18 | Technical Artist / Rigger |
| 19 | Environment Artist |
| 20 | UI/UX Designer |
| 21 | 2D Artist / Graphic Designer |
| 22 | QA Lead / Test Automation |
| 23 | QA Tester / Content Tester |
| 24 | Community Manager / Live Operations |

**Activación inicial:** 02 Product Manager como coordinador; 03 Game Director, 04 Technical Director y 22 QA Lead como tres subagentes. Los otros 20 roles permanecen disponibles bajo demanda.

## Flujo en cada tarea
1. 02 abre ticket con owner único, revisión y criterios observables.
2. 03 revisa la experiencia; 04 revisa contratos si aplica.
3. El coordinador delega tareas independientes a los especialistas y recoge sus HANDOFF.
4. En tareas de implementación, cada agente trabaja en rama/worktree aislado y con archivos propios.
5. 22/23 comprueban evidencias; 02 propone aceptación.
6. CEO aprueba cambios de alcance, releases, gasto y riesgo legal.

## Fuentes de verdad
- Código, contratos y pruebas verificables en el repositorio correspondiente.
- ADR/versiones aprobadas.
- Issues de GitHub y su estado real.
- Documentación del proyecto compartida en los chats.
- Las conversaciones solo aportan contexto y propuestas.

## Repositorios detectados
- Código MMO y PR Tamer: https://github.com/AlpCire/DigimonWorldOnline
- Web: https://github.com/AlpCire/DigimonWorldOnlineWeb (sin cambios desde esta fase)
- Demo digievolución web: https://github.com/AlpCire/scroll-digi-evolution (sin cambios)
También está conectado [DigimonWorldOnline-Launcher](https://github.com/AlpCire/DigimonWorldOnline-Launcher), con código Tauri 2 y README de v0.2.1; su ejecución local no se ha reproducido en esta auditoría. No se ha verificado un repositorio de DigiModelConverter ni el servidor completo en producción.

## Archivos de control
- [Inventario G0](G0_INVENTARIO.md)
- [Backlog G0](G0_BACKLOG.md)
- [Protocolo de HANDOFF](PROTOCOLO_HANDOFF.md)
- [ADR-001 propuesta](ADR-001_no_reescritura.md)

## Reglas de la fase
No reconstruir desde cero, no fusionar el PR Tamer sin revisión, no afirmar la validez de derechos sobre activos de terceros, no desplegar y no modificar cuentas o bases de datos. Realizar primero un corte vertical mínimo y reproducible.

## Seguimiento de la auditoría (2026-10-25)
- [G0-01: revisión técnica Tamer](https://github.com/AlpCire/DigimonWorldOnline/issues/3)
- [G0-02: matriz de QA](https://github.com/AlpCire/DigimonWorldOnline/issues/4)
- [G0-03: inventario completo](https://github.com/AlpCire/DigimonWorldOnline/issues/5)
- [G0-04: futuras evoluciones ramificadas](https://github.com/AlpCire/DigimonWorldOnline/issues/6)
- [Informe técnico independiente](G0_EVIDENCIAS_TAMER.md): 30/30 comprobaciones unitarias C++ con hashes fuente validados; otros entornos pendientes.
- [Matriz de QA](G0_MATRIZ_QA.md): PASS limitado y prerrequisitos para integración.

## Propuesta de primer entregable jugable
- [Vertical slice VS-0/VS-1](G0_VERTICAL_SLICE_PROPUESTA.md): primero loop Tamer reproducible en laboratorio; después una zona corta representativa del Mundo Digital, pendiente de aprobación.

## Codex con 24 roles ejecutables
- [Configuración y piloto multiagente](CODEX_AGENTES.md).
- [Configuración nativa de Codex](../../.codex/config.toml).
- [Agentes especializados](../../.codex/agents): 24 archivos `.toml` cargados bajo demanda por una sesión Codex compatible.
- **No confundir:** los TOML son agentes configurados, no 24 procesos ejecutándose de forma persistente. La coordinación real de tres especialistas se probó el 29/10/2026; la carga automática de estos TOML desde un checkout del repositorio queda por comprobar.
