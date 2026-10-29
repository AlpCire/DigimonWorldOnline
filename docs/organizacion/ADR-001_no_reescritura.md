# ADR-001 — No rehacer el proyecto antes de finalizar G0

- **Estado:** PROPUESTO (requiere aprobación del CEO).
- **Fecha:** 2026-10-25.
- **Contexto:** El repositorio `DigimonWorldOnline` contiene un PR Tamer abierto con módulo C++, addon, scripts, documentación y tests. También hay repositorios web y de interfaz. Se desconoce todavía la localización y el estado verificable del launcher, los terrenos y DigiModelConverter.
- **Decisión propuesta:** conservar intacto el trabajo actual, hacer inventario y validar funcionamiento/reproducibilidad. Evaluar conservar, refactorizar, aislar o reconstruir por módulo.
- **No decidido:** merge del PR #1, lanzamiento, monetización, derechos sobre personajes/modelos, arquitectura definitiva o uso de plataforma de distribución.
- **Alternativas rechazadas provisionalmente:** reinicio total sin auditoría (pérdida de código, pruebas y conocimiento); seguir desarrollando features sin pruebas de integración (aumenta deuda técnica).
- **Consecuencia:** G0 precede a cualquier rediseño irreversible. Todo se documenta mediante ADR, tickets y PRs.
- **Condición de revisión:** completar G0-01/G0-02/G0-03 y disponer de resultados de build y prueba reales.
- **Aprobador:** CEO humano.
