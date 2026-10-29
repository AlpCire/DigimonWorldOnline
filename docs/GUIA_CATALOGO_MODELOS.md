# Catálogo incremental de modelos y grafo de evolución

- `catalog\model_catalog.json` inventaría `DigiModelConverter\output` por
  subcarpetas, registra rutas/tamaños/huellas, aliases y variantes, valida GLB,
  mallas/texturas y busca paquetes de archivos M2/SKIN/BLP. Esto no confirma que
  el servidor haya registrado la especie. El ID interno deriva
  de la clave normalizada y se conserva al reescanear. No es un ID de criatura
  para la base de datos.
- `catalog\evolution_graph.json` conserva ramas, preevoluciones, nivel, origen,
  confianza de la decisión de diseño y disponibilidad. Las conexiones del prompt
  están marcadas `MMO_CUSTOM`; no se atribuye canon o fuente oficial sin verificar.
  Las conexiones sin ambos modelos de WoW y regla operativa no son utilizables.
- Para reescanear nuevos modelos, ejecuta `G:\World of Digimon\04_modelos\catalogo\ACTUALIZAR_CATALOGO.cmd`.
  El escáner conserva notas/revisiones y registros retirados; no cambia originales,
  CSV de juego, bases de datos ni archivos del cliente.
- Comprueba autor y permiso antes de distribuir modelos de terceros.

Estado inicial: 19 carpetas encontradas, 19 fuentes GLB listas, tres paquetes con
M2/SKIN/BLP y 15 conexiones de diseño introducidas en el
prompt. Solo Patamon→Angemon coincide con una regla activa actual. Blast Mode en
nivel 85 está dentro del límite 100 del motor; el grafo lo mantiene pendiente hasta validar los modelos y requisitos. Impmon tiene paquete M2 privado y registro local de prueba; la prueba gráfica sigue pendiente.
