# G0 — Matriz QA / Estado real de pruebas

**Fecha:** 2026-10-25  
**Responsable funcional:** Agente 22 — QA Lead  
**Issue:** https://github.com/AlpCire/DigimonWorldOnline/issues/4

Estados: PASS = ejecutado y resultado registrado; NO EJECUTADO = código/documento disponible pero no ejecutado en esta auditoría; BLOQUEADO = faltan prerrequisitos. Los resultados históricos descritos por PR #1 NO cuentan como PASS nuevos.

| ID | Prioridad | Caso | Estado de esta auditoría | Entorno/prerrequisito | Criterio |
|---|---|---|---|---|---|
| QA-001 | P0 | Política C++ XP, slots, HP, evolución simple | **PASS**: 30/30; Clang 17 y hashes verificados | C++20, sin AzerothCore | Código fuente exacto, `-Wall -Wextra -Werror`, salida PASS |
| QA-002 | P0 | Compilación worldserver AzerothCore + módulo | BLOQUEADO | Core fijado, Docker/Clang/CMake, build | Link correcto, sin errores y artefacto trazable |
| QA-003 | P0 | Migraciones SQL 001–007 en esquema temporal | BLOQUEADO | MySQL laboratorio, copias, PyMySQL | Instalación y repetición sin pérdida de colección |
| QA-004 | P0 | Persistencia: un inicial/colección/activo/XP tras reconectar | BLOQUEADO | worldserver y dos cuentas desechables | Estado antes/después equivalente |
| QA-005 | P0 | Combate: Tamer 0 daño, Digimon causa daño y gana recompensas | BLOQUEADO | worldserver y captura de protocolo | Daño y XP/loot según reglas sin exploit |
| QA-006 | P0 | Evolución simple 21→22 + XP sobrante | BLOQUEADO | fixture con especies nativas y regla temporal | Misma identidad/dueño, XP sobrante y forma |
| QA-007 | P0 | Seguridad: comandos administrativos no accesibles a jugadores | BLOQUEADO | sesiones GM y cuenta normal | Acceso denegado, sin corrupción ni exposición |
| QA-008 | P0 | Captura y rollback de operaciones transaccionales | BLOQUEADO | esquema y E2E desechables | Atomicidad y límites de colección |
| QA-009 | P0 | Barras y teclado UI al entrar/salir de modo Luchar | BLOQUEADO | cliente autorizado 3.3.5a + addon | Sin solapamientos; binds restaurados |
| QA-010 | P1 | Compatibilidad con barras alternativas, otras razas y clases | BLOQUEADO | cliente con matriz de addons y personajes | Sin controles robados ni daño no previsto |
| QA-011 | P1 | Latencia, contención del mutex y carga progresiva | BLOQUEADO | banco SQL/servidor instrumentado | Percentiles y umbrales definidos por 04 y 12 |
| QA-012 | P0 legal | Derechos del pipeline 3D/modelos/cliente | BLOQUEADO | inventario de licencias y asesoramiento | Documentar permisos antes de distribuir |

## Reproducir QA-001
Desde copia de `feature/tamer-alpha-0.3` y fuentes identificadas por SHA:

```sh
clang++ -std=c++20 -Wall -Wextra -Werror tests/unit/policy_test.cpp -o /tmp/digimon-policy
/tmp/digimon-policy
```

SHA esperados: cabecera `34fbfaff70c1ac6752f5224641da1b79fd006754`, test `bff408bf7d6472e548de75d85a1c608e9d8bcace`.
Salida observada: `PASS: 30 independent progression, evolution and slot policy checks`.

## Gate siguiente
No ejecutar SQL, pruebas de protocolo ni instalaciones en servidor real. Solicitar un laboratorio reproducible con backup, secretos solo por variables privadas, controles de acceso y datos desechables. Registrar línea de base, comandos completos, tiempo de ejecución, logs, resultado y regresiones. QA debe revisar PR #1 de forma independiente.
