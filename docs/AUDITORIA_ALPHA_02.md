# Auditoría del paquete Alpha 0.2 — 16/10/2026

Se revisaron los **12 archivos** de `WORLD_DIGIMON_MMO_Alpha_0_2.zip` aportado
por el usuario. Tamaño: 18.828 bytes; contenido descomprimido: 38.125 bytes.
SHA-256 del original:

```text
66d009198de752ce36773de47d70270a66eb7ccb49fbf31a8bf58ee768332bc0
```

Se conserva el original y un inventario con hashes por archivo en el laboratorio
privado. CRC correcto; sin rutas de escape, enlaces simbólicos ni nombres
duplicados. No hay modelos, texturas, cliente, ejecutables ni archivo LICENSE.
Los documentos `TASK_FOR_CODEX.md` y `AGENTS.md` se trataron como antecedentes
del paquete; no sustituyen el encargo actual del usuario.

La comparación usa la Alpha 0.3 de código
`543124dc354206d35e71afd301a5cc10601aeb1f` y AzerothCore
`01339ccfab9cd5d3d28546f79f38ad74d394b148`.

## Hallazgos en el original

**[P1] SQL incompatible con el esquema instalado.** El INSERT de NPC de
`data/sql/db-world/2026_10_16_00_digimon_tamer.sql:20–24` usa `modelid1`.
Esa columna ya no existe en `creature_template`; los modelos residen en
`creature_template_model`. Un EXPLAIN del INSERT sobre una copia desechable
del esquema devuelve **1054: Unknown column 'modelid1'**. No se ejecutó ese
INSERT ni se aplicaron las migraciones antiguas al servidor. En 0.3 el instalador
copia modelos desde las tablas reales y comprueba colisiones de identificadores.

**[P1] Selección y compras sin una operación atómica.** En `src/DigimonTamer.cpp`,
la elección guarda la mascota, inserta el registro y añade dinero por separado
(129–137). La compra aprende el hechizo, inserta, cobra y guarda en operaciones
separadas (204–213), sin comprobar el resultado de la escritura del registro.
Una interrupción o fallo SQL puede dejar estados distintos en moneda, mascota y
tabla de compras. 0.3 utiliza transacciones y tokens del servidor; sus pruebas de
fallo de escritura verifican la reversión. No se ha simulado una caída en el código 0.2.

**[P1] Desactivar el módulo no desactiva su filtro de daño.** Los tres callbacks
de `digi_tamer_no_player_damage` (314–321) no consultan `Digi::Enabled()`.
Con `DigiTamer.Enable=0` seguirían anulando daño de cazadores mientras se cargue
el módulo. El filtro 0.3 comprueba la activación antes de intervenir.

**[P2] SQL de instalación sin repetición segura.** Los INSERT del catálogo y los
NPC son incondicionales (SQL de mundo, 13–24). Reaplicarlos con las mismas claves
produciría duplicados; además 900100 y 900101 ya pertenecen al laboratorio 0.3.
No se deben mezclar ambas instalaciones. 0.3 audita IDs y conserva filas existentes.

**[P2] Vía automática de aprendizaje sin cerrar.** El original solo retira dos
IDs al crear el lobo (127–128); no intercepta los aprendizajes posteriores de la
mascota nativa. El núcleo llama a `InitLevelupSpellsForLevel()` al subir de nivel.
Por tanto, el paquete no garantiza aprendizaje exclusivo mediante entrenador.
La criatura con IA propia de 0.3 no usa esa progresión automática de mascotas.
Este hallazgo es de código; no se ejecutó un ascenso de mascota en 0.2.

**[P2] Configuración que no controla la clase.** `DigiTamer.OnlyHunter` aparece en
el archivo de configuración, pero el C++ no lo lee: `IsTamer()` exige cazador
siempre (32–34). 0.3 sigue el encargo actual y permite gestionar al compañero
independientemente de la clase, con pruebas de guerrero y mago.

Los comprobadores originales también fallan: C++ señala llaves en la misma
línea de los condicionales; SQL señala INSERT sin DELETE previo. No se declara
que el estilo de 0.2 esté aprobado. La excepción de conservación de datos de
0.3 está documentada aparte; no se añade borrado de personajes para satisfacerla.

## Compilación y límites de la evidencia

La comprobación de sintaxis y la **compilación del C++ a objeto temporal pasan**
con Clang 18.1.3 y las opciones reales del núcleo fijado. Hay un aviso por el
parámetro `creature` sin utilizar en la línea 269. No se enlazó un worldserver
0.2 ni se cargó su módulo en el juego; no se certifica el paquete completo.

Las macros antiguas de CMake siguen presentes en la capa de compatibilidad del
núcleo. La macro de configuración emite un aviso de API obsoleta; eso no demuestra
un fallo de compilación. No se ejecutó el CMake aportado.

La lectura de metadatos del mundo confirma la referencia 69 como familia lobo
(1), tipo bestia (1). Esto no certifica su creación ni la barra de mascota en 0.2.
El primer EXPLAIN, protegido por una transacción de solo lectura, fue rechazado
por esa protección (1792); la comprobación posterior sobre un esquema desechable
confirmó el error de columna 1054. La copia de esquema se eliminó después.

El propio paquete advierte que su XP está incompleta: `EnableXPPrototype=0` deja
sin aplicar x0,70. Activarlo recortaría también la XP de la mascota, porque el
núcleo reutiliza el valor modificado en `KillRewarder.cpp:184–188`.
`Pet::GivePetXP` además limita el nivel al del dueño. La progresión propia 0.3
resuelve ambos acoplamientos y se ha probado con compañero nivel 2 y dueño nivel 1.

## Diferencias de diseño conservadas en 0.3

| Sistema | Alpha 0.2 | Alpha 0.3 del laboratorio |
|---|---|---|
| Control | Mascota nativa y barra prevista | IA propia y Gossip; sin barra nativa |
| Moneda | Cobre de WoW; 500 iniciales; precios 100/200 | DigiBits propios; 100 iniciales; precios 20/35 |
| Técnicas | Compra y equipamiento unidos, registro por personaje | Aprendidas/equipadas separadas, por compañero |
| Progreso | XP experimental apagada, nivel ligado al dueño | XP humana x0,70 y progreso independiente |
| Colección | Un único inicial, sin almacén implementado | Colección de 30 por defecto, captura y cambio de activo |
| Ampliaciones | Propuesta de equipo, pociones y varios activos | Estructura preparada; un compañero operativo |

La moneda y capacidad son decisiones provisionales de 0.3, no conversiones
equivalentes de los valores antiguos. El ensayo de tres seguidores pasivos
no habilita tres mascotas combatientes. Evoluciones y DigiPociones siguen pendientes.

## Cobertura de los doce archivos y decisión

Se leyeron README, TASK_FOR_CODEX, AGENTS, CMakeLists, configuración, las dos
migraciones, DESIGN, TAMER_PROGRESSION, TEST_PLAN, el C++ y loader.h. Los planes
de aceptación del ZIP siguen siendo propuestas, no pruebas superadas de 0.2.
Las verificaciones pendientes de clases, grupos, mapas y cliente continúan en
[PRUEBAS_MANUALES.md](PRUEBAS_MANUALES.md).

No existen `digi_tamers`, `digi_learned_moves` ni `digi_attack_catalog` en las bases
del laboratorio inspeccionadas. No hay datos de esas tablas que convertir.
Si se recibe una base antigua, será necesaria una migración explícita por mascota,
propietario y técnicas; no se hará una importación ciega.

Se conserva 0.3 como implementación operativa. No se han copiado bloques de código
del ZIP al repositorio ni publicado el archivo original. Su ausencia de LICENSE
queda registrada y no se atribuye una licencia a ese material. El kit de modelos
original y el cliente autorizado siguen pendientes; no hay un Digimon de terceros certificado.
