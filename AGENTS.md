# AGENTS.md — WORLD DIGIMON MMO

## Ámbito
Estas instrucciones se aplican al repositorio de coordinación y código del MMO. La Constitución operativa, el inventario G0, el backlog y el protocolo de colaboración se encuentran en `docs/organizacion/`. El usuario es el CEO real; el agente 01 solo asesora.

## Orden de autoridad
1. Instrucciones explícitas del usuario y permisos aplicables.
2. Evidencia reproducible del repositorio, compilación, cliente y pruebas.
3. Decisiones aprobadas (ADR) y documentación versionada.
4. Tareas aprobadas y conversaciones.

## Reglas de trabajo
- Antes de modificar, inspecciona código, rama, README, dependencias y pruebas. No asumas que una función descrita está operativa.
- Mantén separados HECHO VERIFICADO, DECLARACIÓN DOCUMENTADA, HIPÓTESIS y PROPUESTA.
- Una tarea debe indicar identificador WDM, propietario, alcance, dependencias, criterios de aceptación, pruebas y reversión.
- Implementa cambios en rama/worktree aislado, con PR y revisión. No edites `main`, no fusiones, no despliegues ni borres datos sin aprobación expresa.
- No fuerces cambios a la rama Tamer `feature/tamer-alpha-0.3`; el PR #1 es trabajo existente que exige revisión antes de integrar.
- No almacenes secretos, credenciales, datos personales, cliente propietario, modelos o texturas sin permisos acreditados.
- Ante problemas de licencia de WoW, Digimon, materiales externos o distribución, detén la acción afectada y eleva decisión al CEO tras asesoría jurídica.
- No proclames tests, arranque, compatibilidad o seguridad sin indicar comando, entorno, fecha y resultado reproducible.
- Si varios agentes quieren modificar los mismos ficheros, el agente 02 debe coordinar propietarios y orden de entrega.
- Ahorra tokens con entregas concretas; no sustituyas verificación por documentación.

## Responsables
- 02 Product Manager: backlog, asignación, coordinación de HANDOFF.
- 03 Game Director + 13 Systems Designer: diseño jugable.
- 04 Technical Director: arquitectura, integración y revisión técnica.
- 05–12: ingeniería, cliente, pipelines, web, launcher, DevOps y seguridad.
- 14–21: narración, niveles, mundos, 3D, UI y arte.
- 22–23: aceptación, regresión y pruebas.
- 24: comunidad y operación.
- 01: asesor del CEO. El CEO real conserva aprobación final.

## Entrega obligatoria
ID WDM, estado, evidencia, archivos afectados, tests y resultados, limitaciones, riesgos, HANDOFF y próxima acción. No simules comunicación con otros chats; deja instrucciones transferibles y enlaza el artefacto compartido.

## Fase vigente: G0
Inventario real y auditoría antes de reescrituras. No se aprueban rediseños irreversibles ni monetización. Ver `docs/organizacion/G0_INVENTARIO.md` y `G0_BACKLOG.md`.
