# Protocolo de comunicación entre los 24 chats

Los agentes son conversaciones consultables, no un equipo con comunicación automática. La coordinación ocurre mediante ticket, archivos versionados y HANDOFF explícito que el usuario o un orquestador compatible traslada.

## Plantilla
```text
HANDOFF: WDM-###-H##
ORIGEN: Agente NN / rol
DESTINO: Agente NN / rol
ESTADO: PROPUESTA | BLOQUEADO | REVISIÓN | VERIFICADO
OBJETIVO:
ALCANCE DENTRO / FUERA:
BASELINE: repo, rama, commit y archivos
EVIDENCIA: rutas, logs, tests, capturas (o NO DISPONIBLE)
DECISIONES APROBADAS: ADR y versión
DEPENDENCIAS:
INPUT:
OUTPUT ESPERADO:
CRITERIOS DE ACEPTACIÓN:
RIESGOS / DERECHOS:
RESPONSABLE DE APROBACIÓN:
```

## Tareas en paralelo
- Nunca dar a dos especialistas permisos para editar el mismo conjunto de archivos simultáneamente.
- Para cada implementación: ticket → rama/worktree → PR → tests → revisión cruzada → decisión.
- Prohibido fusionar a rama por defecto o sincronizada con publicación sin autorización explícita.
- Los chats producen diseños y transferencias; Codex o humanos operan sobre código con las herramientas disponibles.
- Las validaciones no son válidas sin resultados reproducibles.
- Si falta permiso de uso de assets, detener cualquier empaquetado o redistribución de dichos assets.

## Handoff inicial G0-01
ORIGEN: 02 Product Manager
DESTINO: 04 Technical Director
ESTADO: PROPUESTA
OBJETIVO: revisar PR #1 Tamer y producir matriz de riesgos/compatibilidad sin tocar el código.
BASELINE: `AlpCire/DigimonWorldOnline`, PR #1, rama `feature/tamer-alpha-0.3`, commit `b704e89b35d5c9d8c89aa0ef7a749d2bb84f9f34`.
OUTPUT: hallazgos priorizados, mapa de dependencias, pruebas para reproducir, propuesta de integración.
CRITERIO: todos los hallazgos enlazan archivos y diferencian prueba histórica de prueba nueva.
APROBACIÓN: 02 + 22 para criterios técnicos; CEO para decisiones irreversibles.
