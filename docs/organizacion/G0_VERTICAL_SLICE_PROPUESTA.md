# G0 — Propuesta de vertical slice para WORLD DIGIMON MMO

**Estado: PROPUESTA — aprobación del CEO y Game Director pendientes.**  
**Fecha:** 2026-10-25  
**Responsables:** 03 Game Director, 04 Technical Director, 13 Systems Designer, 22 QA Lead.

## Principio
No desarrollar primero decenas de Digimon, mapas y mecánicas. Crear una experiencia mínima reproducible, diferenciando una **demostración técnica** de una **demostración de producto**. El código Alpha Tamer del PR #1 es la base candidata, no una implementación final aprobada.

## Gate VS-0: demostración técnica con activos del laboratorio
**Lugar:** Northshire de prueba, usando recursos nativos que ya referencia el módulo; jamás comercializarlo ni redistribuir archivos de cliente por este documento. No es el mapa final del Mundo Digital.

**Secuencia observable**
1. Iniciar servidor local en instalación de laboratorio, login con cuenta normal.
2. Crear personaje; guía ofrece un compañero de prueba, sin permitir inicial duplicado.
3. Invocar Lobo digital y controlarlo con órdenes desde addon/Dispositivo.
4. Combate: humano no hace daño ofensivo; compañero ataca, gana progreso y recompensa prevista.
5. Aprender y equipar una técnica autorizada y comprobar energía/cooldown.
6. Retirar/invocar; cerrar y reabrir sesión; conservar colección, nivel, XP, salud y técnicas.
7. Verificar evolución simple cruzando umbral usando una regla temporal sobre especies nativas en un entorno desechable; restituir fixture. Sin afirmar que Patamon/Angemon están disponibles.
8. Comprobar que cuenta normal no ejecuta comandos de administrador y que el addon no rompe el teclado al salir de modo Luchar.

**Aceptación de VS-0:** logs, hashes, comando/entorno, vídeo de cliente 3.3.5a, pruebas de seguridad y regresión antes/después del reinicio; cero migraciones en base real; QA-001–QA-009 aplicables en estado PASS o excepciones aprobadas con riesgos documentados.

**Bloqueo actual:** QA unitario 30/30 PASS; build completo, SQL, E2E y cliente gráfico NO verificados en auditoría G0.

## Gate VS-1: experiencia breve representativa del Mundo Digital
**Depende de VS-0 aprobado.** El entorno, assets y lore requieren procedencia clara y revisión jurídica antes de su distribución.

**Alcance propuesto:** una zona pequeña coherente del Mundo Digital (sin expansión masiva de terrenos); introducción clara; un compañero inicial; 3–5 misiones encadenadas; una ruta principal con decisión secundaria de exploración; dos tipos de enemigos; un miniboss; progreso y una decisión de evolución si la arquitectura ramificada y su UI ya están validadas. Duración objetivo a determinar con test de usuarios, no prometer horas no medidas.

**Experiencia del jugador:** entrar, explorar, vincularse con su Digimon, combatir dando órdenes, progresar, realizar una misión significativa, evolucionar y volver al día siguiente con progreso intacto. Debe sentirse distinto de WoW con una skin.

**Exclusiones explícitas de VS-1:** 500 cartas completas, múltiples continentes, 24 líneas evolutivas simultáneas, monetización, economía global, clanes, raids, sistemas masivos de eventos, multiplataforma y un launcher final de producción. Pueden permanecer prototipados, pero no bloquear la vertical slice.

## Matriz de responsabilidades
| Entregable | Responsable único | Revisión |
|---|---|---|
| Alcance y bucle central | 03 Game Director | CEO, 13 |
| Sistema de evolución y reglas | 13 Game Systems Designer | 04, 05, 22 |
| Integración C++/cliente/SQL | 04 Technical Director | 05, 07, 08 |
| Misiones y rutas VS-1 | 14 Quest Designer | 03, 15 |
| Zona de ensayo y materiales | 16 World Builder | 15, 17, 18, 12 |
| HUD/mensajes de combate | 20 UI/UX | 03, 08, 22 |
| Aceptación y ejecución QA | 22 QA Lead | 23 y CEO |
| Riesgos de licencia y distribución | 01 Asesor estratégico | abogado externo + CEO |

## Decisiones pendientes del CEO
- Validar estrategia de no-reescritura global como ADR-001.
- Aprobar VS-0 y VS-1, sus exclusiones y el criterio de paso.
- Priorizar rama evolutiva después de que exista evidencia de VS-0.
- Resolver vía legal la posibilidad de distribución/monetización y derechos de material artístico.
