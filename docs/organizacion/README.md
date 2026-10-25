# Organización multiagente — WORLD DIGIMON MMO

**Estado:** PROPUESTA G0, pendiente de aceptación explícita. **Fecha de inicio:** 2026-10-25.

## Propósito
Coordinar 24 chats especializados con fuentes de verdad verificables. Los chats **no conversan automáticamente entre sí** y su contexto no sustituye a Git. Trabajo de programación mediante Codex/PR; validación real por ejecución de pruebas y revisión humana.

## Jerarquía
CEO real (usuario) → 02 Product Manager (coordinación) → especialistas; 03 Game Director controla coherencia jugable, 04 Technical Director controla contratos técnicos y 22 QA Lead define evidencia de calidad. El asesor 01 no es un segundo CEO.

## Registro de chats (crear manualmente con el kit de prompts)
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

**Activación inicial:** chats 02, 03, 04, 22, más el usuario. El resto bajo demanda; no hay ventaja en ejecutar 24 conversaciones a la vez.

## Flujo en cada tarea
1. 02 abre ticket con owner único, revisión y criterios observables.
2. 03 revisa la experiencia; 04 revisa contratos si aplica.
3. Especialista realiza trabajo o emite HANDOFF documentado.
4. Codex o humano implementa en rama; otro agente no edita a la vez los mismos archivos.
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
No se ha verificado un repositorio accesible del launcher, de DigiModelConverter ni del servidor completo en producción.

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
