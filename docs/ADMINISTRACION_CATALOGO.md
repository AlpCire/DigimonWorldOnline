# Administración, modelos y evolución — Alpha 0.4.0

## Comandos dentro del juego

Requieren cuenta administrador (nivel de seguridad 3); ser dueño de un compañero
o activar el modo GM no concede estos permisos.

| Comando | Resultado |
|---|---|
| `.listadigi` | Lista especies disponibles, pendientes y alias |
| `.spawndigi Nombre [nivel]` | Crea una criatura de prueba delante del administrador |
| `.spawndigi 1 12` | Prueba de la especie ID 1 a nivel 12 |
| `.spawndigi borrar` | Retira las pruebas del administrador en el mapa actual |
| `.digilevel 22` | Ajusta al nivel 22 su compañero activo y guarda el cambio |

Los nombres pueden contener espacios. El último número opcional es el nivel,
entre 1 y 80. Las pruebas caducan a los 180 segundos; máximo cinco por
administrador. También desaparecen al salir su creador del mapa o desconectarse.
Son blancos pasivos para comprobar modelos y combate: no dan botín ni XP, no son
capturables y no añaden filas a la colección. No son compañeros adicionales ni
apariciones permanentes del mundo. No se sustituye un modelo que falte por otro.

`.digilevel` trabaja fuera de combate sobre el compañero activo propio, aunque
esté retirado. Necesita que esté vivo. Pone a cero la XP parcial del nivel ajustado,
aplica una evolución si se ha subido hasta su requisito y registra cuenta,
compañero, forma y niveles anteriores/nuevos en `digimon_admin_audit`. Bajar un
nivel no revierte la forma. No ajusta el nivel del humano.

Tras cambiar los permisos de una cuenta, cerrar sesión y volver a conectar.
Para jugar y recibir recompensas normales con cuenta administradora, usar
`.gm off`: el modo GM activo evita las recompensas del compañero.

## Dónde está cada parte de un Digimon

Una carpeta de modelos no es el catálogo del servidor. Hay tres partes:

1. **Archivos gráficos del cliente:** M2 versión 264, SKIN y texturas BLP, dentro
   de un parche MPQ del cliente. Necesita filas nuevas de CreatureModelData y
   CreatureDisplayInfo. Cada nueva criatura debe usar IDs/rutas propios.
2. **Plantilla del mundo:** `acore_world.creature_template`,
   `creature_template_model` y `creature_model_info`. El módulo usa la IA
   `DigimonCompanionAI`. La plantilla enlaza con el display; no contiene el M2.
3. **Catálogo y progreso:** configuración en `acore_characters.digimon_species`,
   técnicas, asignaciones y reglas; cada jugador tiene sus filas en
   `digimon_collection`, `digimon_active`, `digimon_learned` y `digimon_equipped`.

El repositorio contiene código e identificadores, sin esos archivos gráficos.
Los modelos originales y convertidos se conservan en el laboratorio privado.
Copiar un GLB a una carpeta del servidor no lo convierte en M2 ni crea sus DBC.

## Catálogo editable

`scripts/catalog.py` exporta cinco CSV separados por punto y coma, UTF-8 con BOM
para Excel. Permite validar y aplicar una edición con copia JSON previa de la
configuración. El administrador local puede proporcionar accesos de PowerShell
para estos tres pasos. Exportar a una carpeta nueva para no sobrescribir cambios.

| CSV | Campos principales |
|---|---|
| `digimon_species.csv` | `id`, `name`, `creature_entry`, `stage`, `enabled` |
| `digimon_aliases.csv` | Nombre alternativo y `species_id` |
| `digimon_techniques.csv` | `id`, `name`, `spell_id`, etapa mínima, precio, habilitación |
| `digimon_species_techniques.csv` | Qué `technique_id` puede usar cada `species_id` |
| `digimon_evolution_rules.csv` | Origen, destino, nivel requerido y habilitación |

Etapas: 1 Rookie, 2 Champion, 3 Ultimate, 4 Mega. Usar `enabled=0` mientras falten
recursos. El validador requiere una plantilla real con la IA correcta antes de
habilitar una especie; no instala modelos ni crea plantillas. No eliminar IDs
existentes: se pueden deshabilitar conservando las colecciones.

Para aplicar: cerrar WoW, validar archivos, detener el mundo, aplicar la
configuración y arrancar de nuevo. El catálogo se carga al arrancar, no durante
cada golpe. La importación es una transacción y no modifica filas de progreso.
Un fallo debe resolverse antes de volver a abrir el mundo. El exportador no
sobrescribe archivos existentes. La copia `backup-*.json` es de configuración,
no sustituye al respaldo completo de las bases.

## Patamon → Angemon al nivel 22

El catálogo de ejemplo reserva **Patamon 101** y **Angemon 102**, ambos
deshabilitados hasta instalar sus modelos/plantillas. No se usan animales de WoW
como si fueran esos Digimon. Su regla preparada es:

| source_species | target_species | required_level | enabled |
|---|---|---|---|
| 101 | 102 | 22 | 0 |

Después de instalar y habilitar ambas especies, poner la regla a 1. Patamon al
nivel 21 permanece Patamon; cuando gana XP y alcanza 22 pasa a Angemon conservando
**su ID de colección, dueño, nivel 22, XP sobrante y porcentaje de salud**. Cambian
especie, etapa, aspecto y estadísticas; se mantienen las técnicas aprendidas y
se desequipan las que la nueva especie no admite. No se concede automáticamente
una técnica nueva. El ajuste de nivel de administrador recorre las mismas reglas.

Cada especie tiene una salida automática configurada; un salto de niveles puede
recorrer varias reglas. Cada paso debe subir de etapa, lo que impide ciclos.
Aplicar una regla a un compañero que ya supera el umbral ocurre en su siguiente
subida de nivel; no se cambian todas las colecciones al editar el catálogo.
Las reglas no incorporan por ahora ramificaciones, objetos, requisitos de humano,
pagos, evolución temporal ni reversión. La tabla `digimon_evolutions` de 0.3 sigue
como diseño futuro inactivo: el motor actual usa `digimon_evolution_rules`.

## Técnicas con nombre propio

El nombre se guarda en `digimon_techniques.name`, y la barra y los menús reciben
ese nombre desde el servidor. `spell_id` reutiliza mecánica, animación e icono de
un hechizo instalado. No hace falta distribuir imágenes extraídas del juego:
el cliente ya contiene sus iconos. Varias técnicas pueden reutilizar el mismo
hechizo con nombres distintos y asignarse a especies diferentes; al compartir
spell_id comparten sus recuperaciones nativas. Aprender y equipar siguen separados.

Cambiar un nombre no cambia daño, elemento, efecto o proyectil. El registro de
combate y el nombre de su aura pueden seguir mostrando el hechizo
nativo. Para sustituir también esos nombres se necesitan hechizos propios.
Para una técnica
con mecánica totalmente nueva hacen falta datos/lógica de hechizo adicionales y
posiblemente un parche del cliente; no modificar un hechizo nativo globalmente
para renombrar el ataque de un solo Digimon. Registrar permisos de modelos y
texturas antes de distribuirlos; estar disponibles en Internet no es una licencia.
