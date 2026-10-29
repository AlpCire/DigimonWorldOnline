# Consumibles y cuidado del compañero

Esta entrega añade un catálogo inicial administrado por el servidor. Felicidad se guarda por compañero, empieza en 50/100 y no decae. Empatía continúa siendo un vínculo independiente y tampoco decae. Ninguna de las dos altera todavía el balance de combate.

| Objeto | ID | Efecto |
| --- | ---: | --- |
| Galleta de Datos | 900230 | +15 Felicidad, máximo 100 |
| Ración de Filo | 900231 | +5 Felicidad y +15% daño durante 10 minutos |
| Estofado de Núcleo | 900232 | +5 Felicidad y -15% daño recibido durante 10 minutos |
| Poción de Impulso Digital | 900233 | Velocidad de movimiento del Tamer durante 15 segundos |

Los beneficios de daño y protección se guardan por separado y uno del mismo tipo reemplaza/renueva el anterior. El servidor calcula sus valores y expiración; no se apilan. Para usar comida o raciones se requiere un compañero activo, estar vivo y fuera de combate. La poción usa el hechizo nativo de velocidad del cliente 3.3.5a.

Los tres objetos de cuidado se crean como objetos de inventario de una unidad. El registro temporal del GUID evita duplicar el efecto si el servidor cae entre guardar el beneficio y retirar el objeto; al volver a iniciar sesión, el registro consume el objeto pendiente. El catálogo persistente está en `digimon_consumables`; felicidad y vencimientos están en `digimon_collection`.

## Aplicación

1. Detén `worldserver` antes de actualizar el módulo.
2. Copia `modules/mod-digimon-tamer` y `scripts/install.py` al proyecto AzerothCore.
3. Compila el módulo con la misma configuración del servidor.
4. Ejecuta primero el instalador en modo auditoría. Comprueba la conexión y las colisiones antes de usar la opción `--apply`.
5. Reinicia el servidor y entrega los objetos de prueba con `.additem 900230`, `.additem 900231`, `.additem 900232` y `.additem 900233`.
6. Con un compañero activo, usa cada objeto y comprueba mensajes, felicidad, efecto, expiración y persistencia tras reconectar.

La migración de personajes es `modules/mod-digimon-tamer/sql/characters/021_consumables.sql`. El instalador crea los cuatro objetos de mundo. No se modifica la felicidad o empatía de cuentas existentes aparte del valor inicial 50 para la nueva columna.

## Verificación manual

- Sin compañero, en combate o muerto: comida/raciones se rechazan y no se consumen.
- Comida: felicidad aumenta sin superar 100; tras reconectar conserva el valor.
- Repetir ración del mismo tipo: renueva duración, sin sumar porcentajes.
- Ración de filo y estofado pueden coexistir porque pertenecen a categorías distintas.
- Daño infligido y recibido aplica los porcentajes solo mientras siguen vigentes.
- Muerte, cambio de forma y reconexión conservan cuidado y vencimientos.
- La poción aumenta velocidad por su duración nativa.
- Desactiva `Datos de cuidado`: oculta empatía, felicidad y estado de comida/beneficios en la UI; vuelve a activarlo y verifica que aparecen de nuevo.
- Interrumpe el servidor inmediatamente después de consumir una comida y vuelve a iniciar sesión: el alimento no debe duplicar el efecto ni permanecer en inventario.

La compilación AzerothCore y las pruebas dentro del cliente requieren el entorno local del usuario; este paquete no declara esas pruebas como superadas.
