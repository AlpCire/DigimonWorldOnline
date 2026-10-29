# Ecosistema Digimon y UX

Este trabajo evita fijar rutas evolutivas concretas: las ramas pueden ampliarse sin cambiar la base de objetos, vínculo o interfaz.

## Principios

- El servidor conserva la autoridad sobre inventario, felicidad, comida, efectos, conducta y progresión. La UI presenta estado confirmado y envía órdenes; no concede recompensas localmente.
- Los objetos y efectos se declaran por identificadores estables. Los nombres, iconos, duraciones y valores se pueden ajustar sin acoplarlos a una especie o evolución.
- Cada efecto temporal tiene duración, acumulación explícita, origen y vencimiento persistente o reconstruible. Repetir una comida no debe multiplicar un beneficio sin límite.
- Las acciones visibles muestran coste, requisito, tiempo restante y motivo de bloqueo. La interfaz compacta prioriza compañero, salud/energía, experiencia y controles usados a menudo.

## Ecosistema de objetos

1. **Alimentos de cuidado:** aumentan Felicidad (0–100), una estadística separada de Empatía. Empatía conserva su regla acordada: persiste y no decae. Felicidad tampoco decae en la primera versión; se ajustará solo con pruebas y una decisión de diseño explícita.
2. **Raciones de combate:** pueden aumentar Felicidad y conceder un único beneficio temporal por categoría: daño o protección. Usar otra ración de esa categoría reemplaza o renueva el efecto según la definición del objeto; nunca se apilan de forma ilimitada.
3. **Consumibles del Tamer:** la primera poción usa el hechizo nativo de velocidad del cliente 3.3.5a durante 15 segundos.
4. **Objetos de evolución y misión:** siguen siendo objetos normales de inventario/recompensa. La configuración de una evolución puede requerirlos, pero la comida y los consumibles no dependen de ninguna ruta evolutiva.

Cada entrada de catálogo define ID, categoría, magnitud, duración y hechizo cuando corresponda. Felicidad y Empatía se guardan por Digimon y no se mezclan en fórmulas. El balance y la presentación quedan pendientes de pruebas en juego.

## Interfaz prevista

- HUD compacto con retrato/modelo, nivel, XP actual/requerida, salud, energía y vínculo/felicidad.
- Barra de combate con acciones principales y tres controles pequeños de conducta; el modo activo queda resaltado y se sincroniza desde el servidor.
- Panel de compañero con pestañas cortas: Resumen, Habilidades, Evoluciones, Objetos y Misiones. Una pestaña solo aparece si tiene datos; los detalles secundarios quedan plegados.
- Inventario de compañero y efectos activos muestran icono, nombre, duración y descripción. La UI no inventa resultados mientras espera confirmación del servidor.
- El Digivice pasa a ser un acceso compacto; sus opciones avanzadas no ocupan pantalla de forma permanente.
- El rastreador de encargos se puede ocultar desde Configuración para reducir elementos fijos en pantalla.
- Todos los datos de cuidado (empatía, felicidad, comida y sus efectos) obedecen a un único ajuste de visibilidad. Ocultarlos solo afecta a la presentación; los valores y efectos del servidor siguen activos.

## Estado de implementación

La base inicial está implementada en el candidato 2.2.0: catálogo de cuatro objetos, felicidad persistente, comida, efectos temporales no acumulables, mensaje de estado servidor→addon y visualización controlada por `Datos de cuidado`. La migración de personajes es 021. Falta compilar contra AzerothCore y completar pruebas dentro del cliente antes de dar por validado el comportamiento.

Los valores iniciales son de prueba: +15 felicidad por galleta; raciones con +5 felicidad y 15% de daño o protección por 10 minutos; poción de velocidad por 15 segundos. No se ha añadido decaimiento de felicidad, castigos ni bonificaciones de atributos por felicidad.

## Siguientes sistemas

Tras validar esta base, extender el catálogo con iconos y obtención por misiones, interfaz de inventario de compañero, comida con animaciones y feedback, y balance por etapa. Mantener cada sistema desacoplado de las líneas evolutivas para que nuevas ramas no obliguen a migrar las reglas de objetos.

## Prueba de interfaz de esta entrega

Con la barra de combate abierta, Atacar y Seguir deben mostrar siempre sus iconos. Los botones Agresivo, Defensivo y Pasivo aparecen en la esquina superior derecha de la barra; el seleccionado queda iluminado cuando el servidor confirma el modo. Al cambiar de conducta, comprobar que el texto/estado vuelve a actualizarse tras la respuesta del servidor y después de reconectar.
