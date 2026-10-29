# Validación — 16/10/2026

## Actualización 0.4.1

El límite del Digimon subió a 100 en el núcleo, comandos, catálogo y base de datos; el nivel del Tamer continúa separado. El Dispositivo y el addon muestran las ramas válidas y permiten restaurar formas descubiertas; el servidor registra cambios e historial. AzerothCore Release compiló e inició con la actualización. Pasaron 41 pruebas C++, 20 comprobaciones SQL, 3 pruebas del catálogo y la prueba Lua 5.1 del menú y su selección validada.

Impmon está empaquetado en un MPQ local y registrado como `.spawndigi Impmon`; las formas existentes 60000–60002 se conservaron en cliente y servidor. Falta que el usuario confirme la apariencia dentro del cliente. El origen y permiso del modelo no están documentados; los recursos permanecen privados y fuera del repositorio.

## Administración y evolución 0.4.0

### Extensión del catálogo y grafo — 16/10/2026

El catálogo privado inventaría 19 modelos con IDs estables y conserva anotaciones
manuales; el grafo registra 15 rutas de diseño MMO y marca solo Patamon → Angemon
como conectada al runtime. Impmon se convirtió a M2 v264 (4.313 vértices, 6.954
triángulos, 34 huesos, 12 secuencias, 5 texturas); la validación binaria pasó y
la prueba en WoW y los derechos de origen siguen pendientes. El paquete queda privado.

La migración 008 se ejecutó desde cero, se repitió sin duplicar rutas y pasó 20
comprobaciones SQL en un esquema temporal, incluida una bifurcación y rechazo de
nivel 86. La prueba C++ de política pasó 38 casos. El módulo completo todavía no
se ha recompilado con estos últimos cambios.

Compilación Release sobre el núcleo fijado aprobada. Treinta comprobaciones de
política y evolución; siete migraciones desde cero y quince comprobaciones de
integridad en esquema temporal aprobadas. Reinstalación sobre el laboratorio
ya migrado aprobada sin repetir 007. C++ pasa estilo; SQL mantiene la excepción
documentada de INSERT IGNORE sin DELETE, incluidas las semillas nuevas.

Dos casos de protocolo aprobaron en 38,926 s con cuentas desechables y una regla
temporal 1→2 a nivel 22 usando modelos nativos de prueba. Invocación por nombre,
límite de cinco, retirada, colección intacta, rechazo de modelo no instalado,
niveles 21/22, forma y nivel del actor real, persistencia y rechazo de cuenta
normal comprobados. Una muerte real causada por el compañero sube de 21 a 22,
cambia especie/etapa y conserva la XP sobrante al reconectar. No se presenta como
prueba gráfica de Patamon/Angemon. La regla de ensayo se retiró; su ejemplo real
101→102 queda deshabilitado hasta disponer de modelos.

Regresión del binario final: cuenta normal, dos clases simultáneas y entrenador/
técnicas aprobaron en 12,279 s. Lua 5.1 aprueba la barra y nombres personalizados
recibidos del servidor, incluidos mensajes falsos rechazados y compatibilidad
con los paquetes anteriores. Catálogo CSV exportado, validado y reaplicado sin
cambiar progreso. Los recursos de terceros permanecen fuera del código público.
Tres comprobaciones negativas del importador rechazan especie sin plantilla,
evolución con especies deshabilitadas y nivel requerido inválido, sin aplicar datos.

Los intentos fallidos se conservan: la migración inicial detectó una colisión con
la tabla de diseño futuro de 006, resuelta creando una tabla de reglas distinta;
los primeros casos nuevos leían antes de recibir actualizaciones de objetos y
usaban el identificador fijo del antiguo lobo al buscar la forma nueva. Se
corrigieron las confirmaciones y la consulta de plantilla en el arnés. No se
ocultan esas ejecuciones como aprobadas. No se repitió toda la batería histórica.

## Fila propia de compañero 0.3.3

El vídeo del usuario del 16/10/2026 confirma la carga de un modelo experimental
privado con texturas, participación en combate y progresión del compañero al
nivel 4. No certifica todas las animaciones ni convierte esos recursos en material
redistribuible. El vídeo también demuestra que la superposición 0.3.2 seguía sin
resolver la barra. Se registra ese resultado como fallo visual, aunque la
simulación Lua anterior hubiera aprobado.

La barra se mueve a una fila propia sobre las barras inferiores. Los doce botones
se anclan al panel nuevo, y se retira la ocultación de botones originales. La
prueba Lua comprueba ubicación independiente, controles dentro del panel sin
solapamiento, transparencia original conservada, órdenes y aplazamiento durante
combate. Aprobada en Lua 5.1. La corrección necesita nueva supervisión dentro del cliente gráfico.
No cambia C++, SQL ni modelos; no requiere reiniciar el servidor.

## Corrección de visibilidad 0.3.2

La prueba Lua 5.1 de esa revisión comprobaba capas explícitas de botones y recuperación,
ocultación de los doce botones originales y restauración de su transparencia
individual al salir. Conservaba las comprobaciones de órdenes y cambios pendientes
durante combate. Aprobó en simulación; el vídeo posterior mostró que no resolvía
el problema visual. Se sustituye por la fila propia de 0.3.3.

Se repitieron dos escenarios de protocolo: selección única y reconexión con
guerrero/mago simultáneos; compras, equipo, daño de Mordisco, aura de Aullido,
recuperación, energía y persistencia. Ambos aprobaron en 7,404 segundos. La prueba
de selección verifica además que el paquete de creación del compañero contiene
el display configurado en la base de datos, sin fijar un modelo de terceros en
el código. Se comprobó un display experimental privado; los archivos de ese
modelo no se distribuyen. El programa de pruebas necesitó abrir explícitamente
la conexión de mundo, ausente en la sesión del bot. El fallo previo se conserva
en el laboratorio. Warden se restauró a 1 al terminar.

## Actualización de interfaz 0.3.1

El usuario confirma que el nivel del compañero se conserva tras reconectar en su
cliente. Su vídeo anterior muestra entrada, órdenes, botín y subida del lobo a
nivel 2 con el humano en nivel 1. No se presenta como validación de la barra nueva.

- Compilación Release actualizada y comprobador C++ aprobados. SQL conserva la
  excepción previa de INSERT IGNORE sin DELETE; no cambian las migraciones.
- Sintaxis y contrato de WorldDigimon comprobados con Lua 5.1: posiciones de
  equipo, recuperación, mensajes propios, entrada inválida, órdenes de botones,
  cambio aplazado en combate, restauración de teclas y presentación del objeto.
  Es una simulación de API y no una ejecución del sistema protegido del cliente.
- Prueba de técnicas por protocolo: mensaje real de sincronización con cinco
  espacios, daño de Mordisco 17253 observado en SMSG_SPELLNONMELEEDAMAGELOG,
  aura 24604 de Aullido, rechazo de repetición antes de recuperarse, consumo y
  regeneración de foco, compras/equipo y conservación al reconectar. Aprobada.
- Combate y recompensas, permisos normales, compras inválidas y rollback aprobados
  en la regresión. La primera repetición completa detectó una colisión al crear
  dos guías en el arnés; la prueba se corrigió para compartir un guía. Dos sesiones,
  inicial único, retirada, invocación y reconexión volvieron a aprobar por separado.
- Se corrigió el transporte del estado para usar el opcode normal de mensajes
  incluso cuando el emisor es una cuenta GM de ensayo. El primer fallo de esa
  comprobación se conserva en los registros, junto con las ejecuciones aprobadas.
- Registros privados: `alpha031_pruebas_tecnicas.log` (40,415 s),
  `alpha031_pruebas_regresion_2.log` (cinco casos aprobados y el fallo del arnés),
  `alpha031_pruebas_dos_sesiones.log` (1,838 s). No se presenta la batería con
  un fallo como completamente aprobada.

El dispositivo 900110 se actualizó con el mundo detenido, tras respaldo privado
de las tres bases. La copia se comprobó con gzip y SHA-256; no se ensayó una
restauración. El complemento se instaló con WoW cerrado, guardando su caché de
objetos anterior. Warden vuelve a 1 y las condiciones de captura siguen al 35 %.
La barra nueva, el uso en movimiento y los efectos visuales quedan para supervisión
en el cliente real. No se han vuelto a ejecutar todas las pruebas de captura,
instancias o vehículo para esta actualización.

## Entorno ejecutado

AzerothCore `01339ccfab9cd5d3d28546f79f38ad74d394b148`, Linux Ubuntu 24.04,
Clang 18.1.3, CMake 3.28, compilación Release y módulos estáticos. MySQL 8.4.11.
Sin modificaciones de código del núcleo. Cliente objetivo 3.3.5a / 12340.
Docker Desktop 4.95.0 y WSL 3.0.3.0 sobre Windows.

Las pruebas usan Go 1.26 y AzerothGhost v1.0.8
(`d2e0deefceb3a2d6218216e0da5de4d3d124ee3c`). Autentican, crean personajes y
envían paquetes reales; las verificaciones SQL complementan las observaciones
del protocolo. No se sustituyen errores por resultados positivos.

## Comprobaciones ejecutadas

| Prueba | Evidencia comprobada |
|---|---|
| Compilación | `worldserver` enlazado e instalado contra la revisión indicada |
| SQL desde cero | Seis migraciones ejecutadas en una base temporal; nueve comprobaciones de actualización, inicial único, propiedad, espacios, captura única y equipo aprendido |
| Política de progresión | 23 comprobaciones con `-Wall -Wextra -Werror`; 1000 XP → 700; independencia; etapas; desbordamientos; salud sin pérdida por redondeo |
| Selección e invocación | Guerrero y mago simultáneos; un inicial; clics repetidos; retirada e invocación conservadas al reconectar |
| Combate real | Tres golpes humanos no fallidos con daño cero; Bola de Fuego rechazada; el compañero mata, acredita XP y misión; el dueño abre su ventana real de botín |
| Progresión independiente | Compañero supera al dueño: se prepara en SQL la frontera de XP de una cuenta desechable y una muerte real lo lleva a nivel 2 mientras el dueño sigue a nivel 1 |
| Compra y equipamiento | Solo entrenador; saldo 100 → 80 → 45; aprender no equipa; dos espacios Rookie; aura real 24604; reconexión conserva técnicas |
| Compra adversaria | Paquete repetido, opción inexistente, saldo insuficiente y entrenador fuera de alcance no conceden técnicas ni cobran |
| Fallo de compra | Un trigger limitado a la cuenta de ensayo provoca error; saldo, técnica y marcador de compra revierten juntos |
| Captura | Salud completa y compañero ajeno rechazados; otro jugador no roba el salvaje debilitado; captura única, nivel y salud conservados |
| Colección | Cambio de activo conserva un actor; colección y recuperación del dispositivo persisten al reconectar |
| Fallo de captura | Escritura de colección rechazada deliberadamente: no hay nueva fila, reclamación ni consumo de recuperación; salvaje vivo |
| Capacidad | Colección llena impide captura sin consumir intento |
| Reinicio real | Dos cuentas existentes vuelven a entrar tras reiniciar el proceso; comparación de colección completa, técnicas, activo, salud, nivel, XP, saldo y recuperación |
| Cuenta normal | Sin filas en `account_access`: abre dispositivo, retira e invoca; no accede al ensayo de administrador |
| Ensayo separado | Tres seguidores con GUID y dueño distintos se desplazan; repetir orden no añade más; caducan; el único compañero operativo y los datos persistentes siguen iguales |

La prueba de captura válida usa temporalmente `CaptureChance=100`; comprueba el
camino de éxito de forma determinista. El valor entregado vuelve a **35**. No se
presenta esta prueba como una medición estadística de la probabilidad.
El ensayo de seguidores se ejecuta con `ExperimentsEnable=1` y se entrega con **0**.

Registros de cierre: nueve escenarios de regresión aprobados en 160,279 segundos;
combate y apertura de botín ampliados después, aprobados en 19,937 segundos;
reconexión tras reinicio de ambas cuentas con estado completo, 0,142 segundos.
El ensayo separado y la cuenta normal aprobaron en 32,387 segundos.

## Repetición

Compilar siguiendo `INSTALACION.md`. Para las pruebas de política:

```sh
clang++ -std=c++20 -Wall -Wextra -Werror tests/unit/policy_test.cpp -o /tmp/digimon-policy
/tmp/digimon-policy
```

Con PyMySQL y las variables privadas `DIGIMON_DB_HOST`, `DIGIMON_DB_USER` y
`DIGIMON_DB_PASSWORD`, ejecutar `python3 tests/integration/schema_test.py`.
Crea y elimina exclusivamente una base temporal nueva `digimon_test_*`; requiere
permisos de creación de bases en el servidor de ensayo. No usa las tablas de
personajes reales para preparar los casos inválidos.

Las pruebas de protocolo viven en `tests/e2e`. Necesitan un servidor **desechable**,
Go y variables `E2E_AUTH_ADDR`, `E2E_AUTH_DSN`, `E2E_CHAR_DSN`, `E2E_WORLD_DSN`.
Las DSN se suministran en el entorno privado; nunca se guardan en Git.

```sh
go test -tags=e2e -v -count=1 -timeout 5m -run '^TestTamer_|^TestCapture_' ./...
```

Para captura, detener el mundo, poner temporalmente `CaptureChance=100` y arrancar.
Para el ensayo de seguidores, activar únicamente `ExperimentsEnable=1` y ejecutar
`-run '^TestLab_'` por separado. Restaurar los valores originales al terminar.
Para repetir el reinicio, definir `DIGIMON_RESTART_FIXTURE` con una ruta privada y
`DIGIMON_RESTART_FROM=TestCapture_CollectionOwnershipAndSwitch`; ejecutar captura,
reiniciar realmente el mundo y ejecutar `-run '^TestRestart_'` con
`DIGIMON_RESTART_PROBE=1` y la misma ruta. El archivo contiene identidades de ensayo
y permanece excluido del repositorio.

El reino del laboratorio anuncia `127.0.0.1:8085`. Si los bots se ejecutan en
Docker, comparten el espacio de red del contenedor del mundo; de otro modo esa
dirección apunta al propio contenedor de pruebas. Warden se desactiva solo en este
laboratorio por la autenticación de la herramienta de protocolo; revisar su
configuración antes de usar clientes reales o ampliar acceso.

## Incidencias y límites de la evidencia

La primera batería de captura leyó el paquete de creación anterior a la actualización
de salud. Se corrigió la espera, sin aceptar como válido un resultado incorrecto.
Otra ejecución falló preparando una criatura y otra rechazó un nombre de cuenta
demasiado largo; se acortaron los prefijos y se retiraron únicamente los marcadores
huérfanos de criaturas propias del ensayo. Las baterías fallidas se conservan.
También se corrigió el redondeo de porcentajes para evitar perder HP al reconectar.

Los errores SQL 1644 de los tests de rollback son **provocados deliberadamente**.
El limpiador de AzerothGhost puede intentar borrar una criatura temporal tras
eliminar la fila de un NPC de ensayo; se han observado esos avisos. No se ocultan
como una ejecución sin errores. Eliminar/reutilizar manualmente GUID de criaturas
requiere revisar sus marcadores de reaparición y ciclo de captura.

Los registros completos y las identidades quedan en el laboratorio privado,
en `08_registros/alpha03_*.log`. Los resultados visuales, botín/reputación en grupo,
todas las clases, instancias, latencia, carga y desconexiones abruptas siguen el
protocolo manual. Esas baterías no ejecutan un cliente gráfico. La evidencia
posterior aportada por el usuario para un modelo privado se describe en 0.3.3;
el informe no sustituye las comprobaciones gráficas pendientes.

## Auditoría posterior del ZIP Alpha 0.2

Se revisaron sus doce archivos y hashes. El C++ original supera sintaxis y
compilación a objeto con el núcleo fijado, con un aviso de parámetro sin usar;
no se enlazó ni ejecutó un servidor 0.2. Un EXPLAIN en un esquema desechable
confirma error 1054 por `modelid1`; el INSERT original no se ejecutó. Sus
comprobadores de estilo C++ y SQL fallan. Véase [auditoría](AUDITORIA_ALPHA_02.md).
Esta actualización solo cambia documentación de 0.3: conserva la evidencia de
las pruebas funcionales anteriores y no añade supuestas pruebas del cliente.

## Estilo

El comprobador C++ del núcleo pasa. El comprobador SQL pasa formato, comillas,
terminadores, motor y rutas; señala los `INSERT IGNORE` sin `DELETE` previo.
Es una excepción deliberada que se remite a revisión: borrar filas persistentes
o catálogos al reinstalar sería destructivo. El comprobador conserva su fallo;
no se modifica ni se declara que toda la comprobación SQL está aprobada.
