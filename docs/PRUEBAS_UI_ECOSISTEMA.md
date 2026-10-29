# Pruebas preparadas: barra e interfaz

Usar el cliente WoW 3.3.5a con el addon WorldDigimon actualizado y el servidor del laboratorio.

1. Entrar con un compañero activo, abrir la barra con `/digimon luchar` y confirmar que los iconos de Atacar y Seguir se ven completos.
2. Pasar el cursor sobre cada uno de los tres botones pequeños de conducta. Confirmar que las ayudas muestran Agresivo, Defensivo y Pasivo.
3. Seleccionar cada modo y comprobar que el servidor acepta el comando y resalta el modo activo tras la respuesta. Al reconectar, comprobar que la UI se sincroniza con el modo actual del servidor (que por ahora vuelve a Defensivo al cargar una sesión).
4. En modo Pasivo, confirmar que no inicia ataques; en Defensivo, que responde cuando atacan al Tamer; en Agresivo, que busca objetivos válidos según el sistema actual. Confirmar que ninguna conducta ataca unidades amistosas.
5. Pulsar cada botón durante combate y fuera de combate; revisar que la selección no parpadea ni desplaza los cinco espacios de habilidades.
6. Cambiar resolución y escala de UI, mover la barra si está desbloqueada y confirmar que los tres controles siguen dentro de la barra.
7. Revisar iconos, nombres y acciones en el cliente limpio sin otros addons de barras y con Bartender/Dominos, si se utilizan.
8. En Configuración, desactivar «Datos de cuidado». Empatía debe desaparecer del HUD y Digiequipo; XP y demás datos de progreso deben permanecer. Reactivarlo y comprobar que los valores de cuidado vuelven sin alterarse.
9. Desactivar y reactivar «Rastreador de encargos». El panel fijo debe ocultarse y volver sin perder el encargo activo.

Anotar para cada paso el resultado, la revisión del addon y del servidor, y cualquier error de Lua/chat. No considerar los consumibles del documento de ecosistema implementados hasta que exista código servidor, migración y pruebas propias.
