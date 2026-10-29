# WORLD DIGIMON MMO — agentes Codex y piloto G0

**Fecha:** 29/10/2026. **Rama:** `docs/wdm-001-g0-agent-organization`. **PR:** [#2](https://github.com/AlpCire/DigimonWorldOnline/pull/2), borrador. Esta configuración no está fusionada en `main`.

## Configuración real
- `.codex/agents/` contiene 24 archivos TOML de roles especializados, con `name`, `description` y `developer_instructions`. Se validaron 24 nombres distintos y la presencia de esos tres campos el 29/10/2026. Esta comprobación de estructura no acredita que un cliente local haya cargado los TOML.
- `.codex/config.toml` deja el proyecto en `read-only`, activa subagentes y limita a tres hilos delegados simultáneos. El orquestador principal asume el papel de **02 Product Manager**; los tres especialistas iniciales son `game_director` (03), `technical_director` (04) y `qa_lead` (22).
- Los otros 20 roles son configuraciones disponibles bajo demanda. No ejecutan trabajo ni se comunican por sí solos entre tareas. Los subagentes heredan permisos del hilo principal; sus TOML no crean un sandbox separado.

## Piloto de coordinación ejecutado en esta sesión
Se iniciaron tres subagentes reales de Codex antes de recoger sus respuestas: `/root/game_director`, `/root/technical_director` y `/root/qa_lead`. El hilo principal coordinó sus tareas y recibió tres HANDOFF independientes. La actividad fue de **solo lectura** sobre [PR #2](https://github.com/AlpCire/DigimonWorldOnline/pull/2) y [PR Tamer #1](https://github.com/AlpCire/DigimonWorldOnline/pull/1); no se ejecutaron tests del juego.

| Rol | Evidencia y resultado | Siguiente handoff |
|---|---|---|
| Game Director | Revisó `AGENTS.md` blob `aa99653d9e681de8bb73fcf56538028639681266` y `G0_VERTICAL_SLICE_PROPUESTA.md` blob `fb9f00d9fd7c1abdfb242e0bba26e0ee6c298bd8`. VS-0 es acotada pero sus pruebas de integración y cliente siguen pendientes; VS-1 depende de ese gate y de derechos. | QA convierte la secuencia VS-0 en casos observables; Technical Director revisa factibilidad. |
| Technical Director | PR #1 borrador, SHA `b704e89b35d5c9d8c89aa0ef7a749d2bb84f9f34`; identificó SQL de evolución con una salida por especie y riesgos de escritura síncrona. | Mantener PR #1 aislado; diseñar verificación de build, SQL y concurrencia antes de integrar. |
| QA Lead | `G0_MATRIZ_QA.md` blob `e47cf2222631c9923ccc7e92871cf4641075ff3e` y `G0_EVIDENCIAS_TAMER.md` blob `ed1e1dfb99f44c4ca1b7731003ff4511ee718359`. El 30/30 es resultado documentado del 25/10, **no repetido en este piloto**. QA-002 a QA-012 siguen bloqueadas. | Mantener evidencia histórica separada de nueva ejecución; preparar entorno de laboratorio. |

**Control de baseline:** el head de PR #2 avanzó mientras se inspeccionaba. Cada hallazgo queda anclado en los blobs y SHA anteriores. La prueba acredita delegación y consolidación reales en esta sesión, pero **no valida aún la carga automática de los 24 TOML desde un checkout Git local**. Tampoco acredita build, SQL, cliente, servidor, seguridad ni derechos de distribución. Los cambios posteriores de este PR son de gobernanza y documentación; `main` y la rama Tamer se comprueban antes de cerrar el piloto.

## Activación reproducible en Codex local
1. Clonar este repositorio en una carpeta Git independiente y seleccionar la rama del PR #2:
   ```powershell
   git clone --branch docs/wdm-001-g0-agent-organization https://github.com/AlpCire/DigimonWorldOnline.git
   cd DigimonWorldOnline
   ```
2. Abrir **esa carpeta Git** como proyecto local en Codex. El espejo del proyecto ChatGPT no es este checkout Git. Revisar `AGENTS.md`, `.codex/config.toml` y los TOML antes de confiar en el proyecto.
3. Iniciar una tarea con este encargo: «Actúa como Product Manager. Delega en paralelo una auditoría de solo lectura a `game_director`, `technical_director` y `qa_lead`; espera sus tres resultados, identifica sus hilos, cita SHA y archivos, consolida discrepancias y entrega tres HANDOFF. No modifiques archivos, issues ni ramas, ni fusiones o despliegues».
4. Comprobar en la actividad de Codex que aparecen los tres subagentes con esos nombres y sus respuestas. En CLI, `/subagents` permite inspeccionar hilos cuando está disponible. Registrar versión, error exacto y configuración efectiva si un rol no carga.
5. Antes y después, comprobar `git status --short`, SHA de `main` y de `feature/tamer-alpha-0.3`, y lista de archivos del PR. Un piloto aprobado exige tres handoffs con evidencia y cero cambios funcionales. No fusionar el PR por superar el piloto.

## Delegación posterior
Cada ticket WDM indica propietario de archivos, entrada, salida, dependencias, pruebas y revisión. Para escrituras autorizadas: una rama/worktree por flujo, sin archivos compartidos editados simultáneamente, PR de borrador y revisión cruzada por Technical Director/QA. El CEO humano decide merges, despliegues, publicación de activos, gasto y cambios irreversibles. No se promete autonomía continua: cada tarea requiere una sesión activa o una automatización configurada y supervisada aparte.

## Plus, Work y límites
La [documentación oficial de OpenAI sobre subagentes](https://learn.chatgpt.com/docs/agent-configuration/subagents) indica que Codex puede delegar en paralelo y cargar agentes personalizados de proyecto en clientes locales; ChatGPT Work puede delegar en cuentas elegibles, pero esa ejecución alojada no equivale a cargar estos TOML locales. Los subagentes consumen más uso que una tarea de un solo agente.

Según la [página oficial de precios y uso](https://learn.chatgpt.com/docs/pricing), Plus comparte presupuesto entre Work y Codex; los mensajes dependen del modelo, tamaño de tarea, contexto y si la ejecución es local o cloud. Las cifras publicadas son **estimaciones**, no una cuota fija: para GPT-6.1 Sol, 15–160 mensajes locales por cinco horas; para GPT-6 Luna, 350–3.000. Puede haber límites semanales. Consultar el panel de uso de la cuenta para remanente y reinicio reales. Elegir tres especialistas de lectura para el piloto reduce gasto y conflictos.

Los [worktrees de Codex](https://learn.chatgpt.com/docs/environments/git-worktrees) requieren un repositorio Git local y aíslan checkouts de tareas paralelas. Una automatización programada es un mecanismo aparte; no convierte estos roles en empleados permanentes ni garantiza ejecución cuando el equipo o la app local no están disponibles.
