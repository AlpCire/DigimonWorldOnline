# G0 — Evidencia técnica de inspección independiente del Tamer

**Fecha:** 2026-10-25  
**Estado:** AUDITORÍA PARCIAL (no build del servidor, no SQL real, no cliente gráfico).  
**Fuente:** https://github.com/AlpCire/DigimonWorldOnline/pull/1  
**Rama fuente:** `feature/tamer-alpha-0.3`  
**Commit fuente declarado por PR:** `b704e89b35d5c9d8c89aa0ef7a749d2bb84f9f34`

## 1. Verificaciones originales (ejecutadas en esta auditoría)
Se recuperaron `modules/mod-digimon-tamer/src/DigimonPolicy.h` y `tests/unit/policy_test.cpp` de GitHub (API con ref explícita). Ambos ficheros fueron materializados en un directorio temporal local y se confirmó su identidad por `git hash-object`:

| Ruta | Hash Git de GitHub y copia local |
|---|---|
| `modules/mod-digimon-tamer/src/DigimonPolicy.h` | `34fbfaff70c1ac6752f5224641da1b79fd006754` |
| `tests/unit/policy_test.cpp` | `bff408bf7d6472e548de75d85a1c608e9d8bcace` |

**Herramienta local:** Clang 17.0.0, C++20.  
**Comando (desde raíz de las dos rutas replicadas):**

```sh
clang++ -std=c++20 -Wall -Wextra -Werror tests/unit/policy_test.cpp -o /tmp/policy_test
/tmp/policy_test
```

**Salida exacta:**

```text
PASS: 30 independent progression, evolution and slot policy checks
```

**Interpretación estricta:** pasa el componente de política sin dependencias de AzerothCore: escalado XP, condiciones de evolución simples, nivel máximo, saturación, serialización de salud y límites de técnicas. NO acredita el comportamiento de `DigimonTamer.cpp` unido al servidor, SQL o las operaciones in-game.

## 2. Inspección de código y restricciones de diseño

| ID | Nivel | Observación sustentada | Consecuencia/acción |
|---|---|---|---|
| F01 | LIMITACIÓN FUNCIONAL P1 | `modules/mod-digimon-tamer/sql/characters/007_admin_evolution.sql` usa `source_species` como `PRIMARY KEY` en `digimon_evolution_rules`; `src/DigimonTamer.cpp` declara `std::unordered_map<uint32, Evolution> Evolutions`. | Solo una evolución de salida por especie. Es compatible con la Alpha lineal, incompatible con requisitos futuros de ramificación. Ver issue #6; requiere ADR, migración y tests antes de ampliar contenido. |
| F02 | RIESGO DE ESCALABILIDAD | `src/DigimonTamer.cpp` utiliza `std::recursive_mutex`, `std::unique_ptr<Connection>` y estructuras en memoria; `docs/ARQUITECTURA.md` declara escrituras síncronas. | Medir latencia, concurrencia y bloqueo bajo carga antes de prometer capacidad MMO. No se clasifica aún como defecto. |
| F03 | VALIDACIÓN PENDIENTE | `client/WorldDigimon/WorldDigimon.lua` crea una fila de combate por separado y gestiona bindings y restricciones de combate; `docs/PRUEBAS.md` reconoce ausencia de confirmación gráfica tras revisión 0.3.3. | QA en cliente autorizado 3.3.5a build 12340, con UI estándar y variantes de addon. |
| F04 | INTEGRACIÓN PENDIENTE | `main` contiene solo README; código Tamer está en PR #1 aún abierto. | No fusionar sin revisión de SQL/migraciones, build y regresión. |
| F05 | LICENCIAS PENDIENTES | `docs/PERMISOS_RECURSOS.md` no acredita permiso de redistribución sobre modelos externos; repo no incluye cliente, texturas ni DBC. | Bloquear empaquetado/publicación de materiales no autorizados; elevar revisión jurídica. |
| F06 | CALIDAD PENDIENTE | `docs/PRUEBAS.md` enumera pruebas históricas, pero registros completos residen en laboratorio privado y no se han reproducido aquí. | No convertir esas afirmaciones en nuevos PASS. Reproducir por comandos de QA, y documentar entorno/logs. |

## 3. Estado de validaciones en esta fase

| Categoría | Resultado G0 |
|---|---|
| Test de política C++ aislado | **PASS** (30/30, hash exacto) |
| Build completo de worldserver | NO EJECUTADO: falta entorno AzerothCore compatible |
| Migraciones MySQL 001–007 | NO EJECUTADO: no se ha facilitado base temporal de laboratorio |
| E2E de protocolo Go | NO EJECUTADO: falta servidor de ensayo |
| Test Lua 5.1 | NO EJECUTADO: no se dispone de runtime Lua 5.1 en el entorno de esta ejecución |
| Prueba real de cliente 3.3.5a | NO EJECUTADO: cliente no disponible ni debe subirse al repositorio |
| Test de carga/concurrencia | NO EJECUTADO |
| Licencias de Digimon y recursos externos | NO CERTIFICADAS |

## 4. Prioridad recomendada
1. Cerrar la auditoría de integración y migraciones en #3.
2. Construir matriz de QA ejecutable en #4, primero en laboratorio desechable.
3. Completar inventario y rutas de recursos en #5.
4. Diseñar ramificación del árbol de evolución en #6 (sin implementarla durante G0).

## 5. Decisión
Conservar el código de la Alpha y realizar un gate técnico antes de fusionar o refactorizar. Esta recomendación no constituye aprobación del PR #1 ni certificación de producción.
