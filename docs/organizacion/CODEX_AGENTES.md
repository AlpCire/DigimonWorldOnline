# WORLD DIGIMON MMO — Subagentes reales de Codex

**Configuración inicial:** 2026-10-29. **Estado:** archivos creados, piloto en Codex aún NO ejecutado. **Rama de preparación:** `docs/wdm-001-g0-agent-organization` y PR #2.

## Qué está configurado
- `.codex/config.toml`: delegación habilitada, límite de **4 subagentes concurrentes** más el orquestador, confirmación interactiva y sandbox `workspace-write`.
- `.codex/agents/*.toml`: **24 agentes especializados** con nombres, descripciones e instrucciones efectivas de Codex.
- `AGENTS.md`: orquestación desde hilo principal, tareas independientes, límites de seguridad y control del CEO.
- `docs/organizacion/`: backlog, tests, inventario, HANDOFF y propuesta de vertical slice.

**Importante:** los archivos registran roles disponibles. No ejecutan 24 procesos hasta que el usuario inicia tareas en Codex; una tarea puede generar subagentes bajo demanda. Work puede delegar subagentes en la nube, pero las configuraciones personalizadas `.codex/agents` se orientan a Codex local.

## Inicio desde la aplicación de escritorio (Plus elegible)
1. Abrir ChatGPT para escritorio y seleccionar **Codex** en el menú.
2. Clonar el repositorio con Git en una carpeta de trabajo nueva (recomendado para no mezclar con el laboratorio local):
   ```sh
   git clone --branch docs/wdm-001-g0-agent-organization https://github.com/AlpCire/DigimonWorldOnline.git
   cd DigimonWorldOnline
   ```
3. Abrir esa carpeta local en Codex y marcarla como confiable solo si se ha revisado su contenido.
4. Confirmar en la configuración que se lee `.codex/config.toml` (versiones Codex recientes), que están accesibles los roles y que el permiso de ejecución del padre no permite acciones no deseadas.
5. Iniciar el **piloto de solo lectura** con el prompt inferior. Ver en la vista de actividad cómo Codex abre subagentes y recoge sus respuestas.
6. No fusionar PR #2 ni PR #1 solo por superar este piloto; se debe revisar su contenido y ejecutar después las pruebas completas.

En CLI, desde la carpeta clonada, ejecutar `codex`; una vez dentro se puede inspeccionar actividad de subagentes con `/subagents` si la versión instalada lo permite.

## Prompt de arranque piloto G0 (copiar al hilo principal de Codex)

> Eres el ORQUESTADOR principal de WORLD DIGIMON MMO. Lee AGENTS.md, .codex/config.toml y docs/organizacion. No modifiques archivos del juego ni hagas commits, pushes, merges, despliegues ni instalaciones. Ejecuta **una delegación real y observable** en paralelo a tres subagentes Codex personalizados: `technical_director`, `sql_engineer` y `qa_lead`. Todos deben limitarse a investigar. 
>
> * `technical_director`: revisa la arquitectura del módulo Tamer del PR #1, relación con AzerothCore y dependencias de compilación.
> * `sql_engineer`: revisa migraciones 001–007, integridad, idempotencia, transacciones y limitación de evoluciones ramificadas.
> * `qa_lead`: compara el alcance de las pruebas existentes con los riesgos reales y propone un gate reproducible sin afirmar nuevos PASS.
>
> Los archivos del Tamer están en `origin/feature/tamer-alpha-0.3`, NO en la rama actual de documentación. Usa `git show origin/feature/tamer-alpha-0.3:RUTA` para inspección; si falta la referencia, solicita permiso antes de operaciones de red. Limita el pilotaje a lectura y análisis.
>
> Espera a que los tres subagentes terminen, contrasta hallazgos y presenta: (1) quién actuó y qué examinó; (2) evidencias exactas con archivos y commits; (3) discrepancias; (4) riesgos P0/P1/P2; (5) propuesta de tickets para revisión del CEO. No cambies los tickets todavía y NO afirmes haber ejecutado un agente que no haya aparecido realmente en la actividad.

## Escenario futuro de implementación
El hilo principal puede llamar `product_manager`, `systems_designer`, `cpp_gameplay`, `sql_engineer` y `qa_lead` por etapas, en lugar de todos a la vez. Primero diseño y contratos; después C++/SQL en ramas o worktrees independientes; finalmente QA. No trabajar en paralelo sobre los mismos archivos.

## Controles de seguridad
- La instrucción de solo lectura del piloto es **una regla de trabajo**, no una barrera de permisos. Para impedir técnicamente cualquier edición, seleccionar modo de permiso de lectura si está disponible o ejecutar Codex CLI con sandbox de solo lectura para ese ensayo. La configuración por defecto del proyecto es `workspace-write` para permitir futuras implementaciones, siempre con aprobación.
- Los subagentes heredan permisos y sandbox del hilo principal. Sus TOML no anulan estos controles.
- No se han configurado claves ni servicios MCP adicionales.
- Nunca copiar datos privados, credenciales de servidor, modelos de origen dudoso, archivos de cliente WoW, DBC o texturas a Git.
- Límites de Plus y créditos pueden restringir la concurrencia efectiva; 4 es el máximo configurado, no una garantía de ejecución constante.
- No se ha ejecutado Codex multiagente real desde esta conversación. La validación real ocurre cuando se abra Codex y se vean los hilos delegados.

## Criterio de éxito del piloto
Al menos tres subagentes visibles y identificables, cada uno con resultados diferentes y evidencia respaldada por el código, el orquestador consolida, no hay modificación de archivos funcionales, no se crea ningún merge ni despliegue y quedan explícitos los pasos pendientes. Si falla, registrar versión de Codex y texto exacto del error antes de modificar configuración.

## Referencias
- Documentación oficial: https://developers.openai.com/codex/subagents
- Configuración: https://developers.openai.com/codex/config-reference
- PR del arranque: https://github.com/AlpCire/DigimonWorldOnline/pull/2
- PR Tamer: https://github.com/AlpCire/DigimonWorldOnline/pull/1
