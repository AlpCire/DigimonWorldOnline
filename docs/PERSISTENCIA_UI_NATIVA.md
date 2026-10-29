# Persistencia de interfaz sin addon — diseño de respaldo

## Decisión pendiente de la prueba

El candidato `patch-esES-b.NATIVE-SV-EXPERIMENTAL.MPQ` declara `WorldDigimonDB` como variable por personaje en `FrameXML.toc`. Hay que comprobar dos cosas en el cliente aislado: que el parche carga y que, después de mover una ventana y descubrir una celda de mapa, ambas decisiones permanecen al salir por completo y volver a entrar. Hasta observarlo, no se puede contar con ese mecanismo.

Si no persiste, la alternativa es guardar en `acore_characters` únicamente las preferencias de presentación y las celdas descubiertas. Salud, XP, talentos, equipo, cajas y evolución ya pertenecen al servidor y no se duplicarán.

## Contrato propuesto

- Tabla de preferencias por `owner_guid`, con esquema versionado. Solo claves conocidas; los números se acotan y las posiciones se limitan al área útil de pantalla. Nunca aceptar Lua, comandos ni cadenas arbitrarias del cliente.
- Tabla de descubrimiento por `owner_guid`, mapa y celda (`DWCContinent` y `DWCZone01`…`09`, 96 celdas por mapa). Clave única para que repetir mensajes sea idempotente.
- Al entrar, el servidor envía un conjunto compacto de preferencias y diez máscaras de 96 bits. La interfaz pinta el mapa cuando termina esa sincronización. No se confía en un valor local como fuente de verdad.
- Al cambiar una preferencia o descubrir una celda, el cliente envía solo el cambio; el servidor valida identificadores y rangos antes de persistir. No enviar la tabla completa en cada fotograma.
- Durante la transición, el addon y el cliente integrado consumirán el mismo protocolo. La importación inicial del `WorldDigimonDB` existente será una operación única, marcada en servidor, para no perder el progreso visual del usuario.
- Las celdas de mapa no otorgan XP, moneda ni desbloqueos. La persistencia de mapa es visual; cualquier recompensa futura por exploración tendrá validación de posición en servidor.

## Pruebas de aceptación

1. Primer inicio sin datos: ajustes por defecto y mapa cubierto.
2. Cambio de escala, opacidad, visibilidad y posición: se conserva tras `/reload`, desconexión y cierre completo.
3. Una celda revelada permanece y no descubre otras zonas por error.
4. Cambiar de personaje no mezcla preferencias ni celdas.
5. Dos clientes con la misma cuenta no pueden escribir sobre otro personaje.
6. Mensajes repetidos, fuera de rango o malformados se ignoran sin bloquear la interfaz ni generar escrituras ilimitadas.
7. La importación desde el addon conserva datos existentes y no vuelve a ejecutarse una vez marcada.

No aplicar una migración SQL ni retirar el addon hasta que la prueba de FrameXML determine si esta ruta es necesaria.
