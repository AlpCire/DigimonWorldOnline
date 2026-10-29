# Protocolo de coordinación de subagentes Codex

## Mecanismo
El hilo principal de Codex actúa como Product Manager y orquestador. Puede iniciar subagentes especializados, enviarles instrucciones y esperar sus resultados **dentro de una tarea activa**. Los 24 archivos TOML definen roles disponibles; no son procesos permanentes ni 24 chats que se llamen solos. Issues, PR, ADR y este protocolo guardan el contexto entre tareas.

## Asignación
1. Registrar ID WDM, objetivo, rama y SHA de partida, propietario único, archivos o área, dependencias, criterio de aceptación y evidencia esperada.
2. Delegar solo partes independientes. En G0: coordinador más `game_director`, `technical_director` y `qa_lead`, con un máximo inicial de tres subagentes simultáneos.
3. El coordinador espera los tres resultados, resuelve divergencias, separa hechos de propuestas y registra una decisión o siguiente ticket. Un subagente no da por aprobada la tarea.
4. En auditoría se trabaja en solo lectura. Para implementación posterior, un agente propietario por conjunto de archivos y una rama/worktree por flujo de escritura; no compartir el mismo checkout mutable ni editar archivos coincidentes a la vez.
5. Integración: PR de borrador → evidencia de pruebas → revisión cruzada técnica/QA → decisión humana. No fusionar a `main`, desplegar, borrar datos ni publicar activos sin autorización explícita del CEO.

## HANDOFF obligatorio
```text
HANDOFF: WDM-###-H##
ORIGEN / DESTINO: rol e identificador de tarea
ESTADO: PROPUESTA | BLOQUEADO | REVISIÓN | VERIFICADO
OBJETIVO / ALCANCE DENTRO Y FUERA:
BASELINE: repositorio, rama, commit SHA, archivos
EVIDENCIA: URL/ruta, comandos, entorno, fecha, resultado; o NO DISPONIBLE
HALLAZGOS: HECHO VERIFICADO | DECLARACIÓN DOCUMENTADA | HIPÓTESIS | PROPUESTA
DEPENDENCIAS / RIESGOS / DERECHOS:
OUTPUT Y CRITERIOS DE ACEPTACIÓN:
ARCHIVOS MODIFICADOS Y PRUEBAS EJECUTADAS:
APROBACIÓN PENDIENTE / PRÓXIMA ACCIÓN:
```

## Handoff inicial G0-01
**Origen:** Product Manager. **Destino:** Technical Director. **Estado:** propuesta de auditoría. Revisar PR Tamer [#1](https://github.com/AlpCire/DigimonWorldOnline/pull/1) en `feature/tamer-alpha-0.3`, SHA `b704e89b35d5c9d8c89aa0ef7a749d2bb84f9f34`, sin tocar código. Entregar riesgos priorizados, mapa de dependencias y pruebas por reproducir. Distinguir resultados históricos de los ejecutados en la tarea. QA revisa criterios; el CEO decide integración o cambios irreversibles.

El piloto de coordinación realizado el 29/10/2026 y sus límites están en [CODEX_AGENTES.md](CODEX_AGENTES.md).
