# Catálogo de ataques de la línea de Impmon: iconos y efectos WoW 3.3.5a

## Resultado de la auditoría

El catálogo local contiene **70 técnicas**, asignadas a **15 formas**: Pagumon, Impmon, Wizardmon, Beelzemon, Beelzemon Blast Mode, Devimon, Myotismon, VenomMyotismon, MaloMyotismon, Bakemon, Phantomon, Piedmon, Witchmon, LadyDevimon y Lilithmon. Las ramas salen de Impmon hacia Wizardmon, Devimon, Bakemon y Witchmon; las evoluciones posteriores están registradas en el grafo del módulo. La migración las mantiene desactivadas (`enabled=0`) hasta que la plantilla/especie está habilitada.

Las técnicas ya no son 70 hechizos nuevos: cada técnica tiene nombre, desbloqueo, coste/enfriamiento, perfil de combate y una referencia a un hechizo existente. Los 70 perfiles contemplan ataques directos, en área/conos/suelo, cargas/pasos, canales, golpes repetidos y potenciadores. El código llama `CastCustomSpell` con esos IDs; el cliente toma el visual que el DBC de esa entrada ya define. El inventario estático de referencias visuales es: `17` (2 usos de perfil), `172` (1 usos de perfil), `403` (6 usos de perfil), `686` (20 usos de perfil), `689` (2 usos de perfil), `702` (5 usos de perfil), `770` (1 usos de perfil), `853` (1 usos de perfil), `980` (1 usos de perfil), `1079` (1 usos de perfil), `1490` (3 usos de perfil), `1719` (1 usos de perfil), `2094` (6 usos de perfil), `2136` (9 usos de perfil), `4286` (6 usos de perfil), `5277` (1 usos de perfil), `5782` (1 usos de perfil), `6660` (3 usos de perfil), `16827` (8 usos de perfil), `26297` (1 usos de perfil), `30455` (3 usos de perfil), `33702` (2 usos de perfil).

Esto aprovecha los impactos, proyectiles, canales, auras y desplazamientos que ya trae el cliente 3.3.5a. Cada ataque mantiene números/alcance/área propios; el ID nativo aporta el envoltorio visual. Los perfiles ya combinan algunos spell visuales secundarios/utility para efectos compuestos. No añadí un VFX 3D externo: un efecto genuinamente nuevo necesita editar Spell.dbc/SpellVisual.dbc y distribuir una actualización de cliente coherente; el addon por sí solo no puede crear esa clase de geometría/partículas. Las referencias `SpellVisualID_1/2` y `SpellIconID` provienen de los DBC del cliente según la documentación de AzerothCore [Spell.dbc](https://www.azerothcore.org/wiki/spell) y [spell_dbc](https://www.azerothcore.org/wiki/spell_dbc).

## Cambios preparados en esta entrega

- Cinco iconos propios, coherentes y legibles en 32–36 px: físico, fuego, sombra, naturaleza/veneno y arcano. Todos se guardan como TGA de 64×64 para el addon 3.3.5a; las ilustraciones PNG fuente quedan en `art-source/` del paquete de trabajo.
- El addon selecciona el icono por ID de técnica, no por el hechizo nativo que se reutiliza. El resto de especies mantiene su icono habitual.
- Se incorpora la tabla de las 70 técnicas con forma, nivel, categoría, forma del perfil y hechizos primario/secundario/utilidad, para verificar en cliente las combinaciones reales.

## Revisión visual y pendientes de prueba

Los efectos que ya existen en WoW se conservan y se reutilizan por `primary_spell`, `secondary_spell` y `utility_spell`. El inventario numérico de abajo sale de los perfiles del módulo, no de una captura del cliente. Antes de publicar nuevas SpellVisual/DBC, hay que probar las cinco familias y varias combinaciones representativas in-game y confirmar que el cliente build 12340 muestra el efecto esperado sin crash ni visual ajeno (p. ej. una animación de disparo humano en un ataque de Beelzemon). No se puede certificar el aspecto sólo con el hecho de que el servidor valide el ID.

En el cliente se pueden customizar iconos por TGA y nombres/tooltip por el addon. El color y las animaciones de los hechizos nativos no se sustituyen por iconos; los VFX totalmente propios quedan como una fase de cliente parcheado y requieren un MPQ privado, testado con el build exacto y no distribuido junto al código público.

## Técnicas, perfiles y referencias nativas

| ID | Forma | Técnica | Nv. | Tipo | Perfil | Spell visual primario | Secundario | Utilidad | Icono propio |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 1000 | Pagumon | Burbujas venenosas | 5 | NATURE | SINGLE | 4286 | — | — | nature |
| 1001 | Pagumon | Burbujas ácidas | 7 | NATURE | SINGLE | 403 | 770 | — | nature |
| 1002 | Pagumon | Embestida | 9 | PHYSICAL | SINGLE | 16827 | — | — | physical |
| 1003 | Impmon | Noche de fuego | 10 | FIRE | SINGLE | 2136 | — | — | fire |
| 1004 | Impmon | Invocación | 13 | FIRE | SINGLE | 2136 | 30455 | — | fire |
| 1005 | Impmon | Embestida nocturna | 17 | PHYSICAL | DASH | 2136 | — | — | physical |
| 1006 | Impmon | Pilar de fuego | 20 | FIRE | GROUND | 2136 | — | — | fire |
| 1007 | Impmon | Marca traviesa | 23 | SHADOW | SINGLE | 1490 | — | — | shadow |
| 1008 | Wizardmon | Nube eléctrica | 25 | NATURE | SINGLE | 403 | — | — | nature |
| 1009 | Wizardmon | Juego mágico | 29 | SHADOW | SINGLE | 686 | 2094 | — | shadow |
| 1010 | Wizardmon | Ilusión aterradora | 34 | SHADOW | SINGLE | 5782 | — | — | shadow |
| 1011 | Wizardmon | Barrera arcana | 40 | ARCANE | SELF | 17 | — | — | arcane |
| 1012 | Wizardmon | Tormenta eléctrica | 50 | NATURE | AREA | 403 | — | — | nature |
| 1013 | Wizardmon | Magia prohibida | 60 | ARCANE | SELF | 33702 | — | — | arcane |
| 1014 | Wizardmon | Ritual abisal | 68 | SHADOW | CHANNEL | 686 | — | — | shadow |
| 1015 | Beelzemon | Impacto doble | 70 | PHYSICAL | SINGLE | 6660 | — | — | physical |
| 1016 | Beelzemon | Garra de oscuridad | 72 | PHYSICAL | SINGLE | 16827 | 1079 | — | physical |
| 1017 | Beelzemon | Disparo rápido | 75 | SHADOW | SINGLE | 6660 | — | — | shadow |
| 1018 | Beelzemon | Disparo desgarrador | 78 | SHADOW | SINGLE | 6660 | — | — | shadow |
| 1019 | Beelzemon | Carga Behemoth | 80 | PHYSICAL | DASH | 16827 | — | — | physical |
| 1020 | Beelzemon | Furia del señor demonio | 83 | ARCANE | SELF | 1719 | — | 26297 | arcane |
| 1021 | Beelzemon Blast Mode | Disparo mortal | 85 | SHADOW | SINGLE | 686 | — | — | shadow |
| 1022 | Beelzemon Blast Mode | Estallido del caos | 87 | PHYSICAL | AREA | 686 | — | — | physical |
| 1023 | Beelzemon Blast Mode | Impacto doble: ráfaga | 89 | SHADOW | SINGLE | 2136 | — | — | shadow |
| 1024 | Beelzemon Blast Mode | Golpe del león espiritual | 90 | ARCANE | SINGLE | 403 | — | — | arcane |
| 1025 | Beelzemon Blast Mode | Aniquilación final | 90 | SHADOW | AREA | 2136 | 686 | — | shadow |
| 1026 | Devimon | Garra mortal | 25 | PHYSICAL | SINGLE | 16827 | 686 | — | physical |
| 1027 | Devimon | Alas de cuero | 29 | PHYSICAL | CONE | 686 | — | — | physical |
| 1028 | Devimon | Contrato infernal | 34 | SHADOW | SINGLE | 702 | — | — | shadow |
| 1029 | Devimon | Uña mortal | 40 | PHYSICAL | SINGLE | 16827 | — | — | physical |
| 1030 | Myotismon | Asalto nocturno | 45 | SHADOW | AREA | 686 | — | — | shadow |
| 1031 | Myotismon | Corriente sangrienta | 49 | SHADOW | SINGLE | 686 | — | — | shadow |
| 1032 | Myotismon | Grito mortal | 53 | SHADOW | SINGLE | 853 | — | — | shadow |
| 1033 | Myotismon | Ola de pesadilla | 58 | SHADOW | SINGLE | 2094 | — | — | shadow |
| 1034 | Myotismon | Drenaje vampírico | 62 | SHADOW | CHANNEL | 689 | — | — | shadow |
| 1035 | VenomMyotismon | Infusión de veneno | 65 | NATURE | SINGLE | 4286 | — | — | nature |
| 1036 | VenomMyotismon | Llama del caos | 69 | FIRE | CONE | 686 | — | — | fire |
| 1037 | VenomMyotismon | Salvajismo tirano | 73 | PHYSICAL | SINGLE | 16827 | — | — | physical |
| 1038 | VenomMyotismon | Infierno | 78 | FIRE | CONE | 2136 | — | — | fire |
| 1039 | MaloMyotismon | Llama del pandemónium | 80 | FIRE | GROUND | 2136 | — | — | fire |
| 1040 | MaloMyotismon | Sangre fundida | 83 | SHADOW | SINGLE | 686 | 172 | — | shadow |
| 1041 | MaloMyotismon | Dominio de oscuridad | 86 | SHADOW | AREA | 702 | 1490 | — | shadow |
| 1042 | MaloMyotismon | Llama del apocalipsis | 90 | FIRE | AREA | 2136 | 686 | — | fire |
| 1043 | Bakemon | Mano infernal | 25 | SHADOW | SINGLE | 686 | — | — | shadow |
| 1044 | Bakemon | Talismán mortal | 29 | SHADOW | CHANNEL | 689 | 702 | — | shadow |
| 1045 | Bakemon | Ola de pesadilla | 34 | SHADOW | SINGLE | 2094 | — | — | shadow |
| 1046 | Bakemon | Aliento venenoso | 40 | NATURE | CONE | 4286 | — | — | nature |
| 1047 | Phantomon | Segador de almas | 45 | PHYSICAL | SINGLE | 16827 | — | — | physical |
| 1048 | Phantomon | Sentencia de muerte | 50 | SHADOW | DELAY | 686 | — | — | shadow |
| 1049 | Phantomon | Estrella diabólica | 55 | SHADOW | SINGLE | 686 | — | — | shadow |
| 1050 | Phantomon | Paso fantasma | 60 | PHYSICAL | STEP | — | — | — | physical |
| 1051 | Piedmon | Espada Trump | 65 | PHYSICAL | SINGLE | 30455 | — | — | physical |
| 1052 | Piedmon | Truco de payaso | 69 | SHADOW | SINGLE | 686 | 2094 | — | shadow |
| 1053 | Piedmon | Cuadrado de máscaras | 73 | PHYSICAL | STEP | — | — | 5277 | physical |
| 1054 | Piedmon | Maravilla de juguete | 78 | ARCANE | CONE | 403 | — | — | arcane |
| 1055 | Piedmon | Teatro del caos | 80 | SHADOW | AREA | 702 | 2094 | — | shadow |
| 1056 | Witchmon | Presión acuática | 25 | SHADOW | SINGLE | 30455 | — | — | shadow |
| 1057 | Witchmon | Vendaval lunar | 29 | ARCANE | SINGLE | 403 | — | — | arcane |
| 1058 | Witchmon | Tormenta venenosa | 34 | NATURE | AREA | 4286 | — | — | nature |
| 1059 | Witchmon | Barrera de bruja | 40 | ARCANE | SELF | 17 | — | — | arcane |
| 1060 | LadyDevimon | Ola de oscuridad | 45 | SHADOW | CONE | 686 | — | — | shadow |
| 1061 | LadyDevimon | Lanza de oscuridad | 49 | SHADOW | SINGLE | 686 | — | — | shadow |
| 1062 | LadyDevimon | Veneno | 53 | NATURE | SINGLE | 4286 | — | — | nature |
| 1063 | LadyDevimon | Seducción oscura | 58 | SHADOW | SINGLE | 702 | — | — | shadow |
| 1064 | LadyDevimon | Alas de pesadilla | 62 | SHADOW | AREA | 686 | — | — | shadow |
| 1065 | Lilithmon | Dolor fantasma | 65 | SHADOW | SINGLE | 686 | — | — | shadow |
| 1066 | Lilithmon | Uña Nazar | 69 | NATURE | SINGLE | 16827 | 4286 | — | nature |
| 1067 | Lilithmon | Tentación oscura | 73 | SHADOW | SINGLE | 2094 | — | — | shadow |
| 1068 | Lilithmon | Reina de la oscuridad | 78 | ARCANE | SELF | 33702 | — | — | arcane |
| 1069 | Lilithmon | Siete pecados | 80 | SHADOW | AREA | 980 | 1490 | — | shadow |

## Prueba manual al instalar

1. Cerrar WoW, instalar el addon actualizado y confirmar que las cinco texturas TGA se cargan (sin icono `?`).
2. Invocar Impmon y equipar Burbujas venenosas, Noche de fuego, Marca traviesa y Embestida. Confirmar iconos propios, nombre/tooltip, cooldown y efecto nativo correspondiente.
3. Probar un ataque de cada tipo: físico, fuego, sombra, naturaleza y arcano; hacer capturas y apuntar técnica/forma, si el visual es correcto, si acierta/daña y si hay una animación impropia.
4. Probar un multiobjetivo, un canal y una carga/paso: confirmar que el visual termina sobre el Digimon/objetivo correcto y no sobre el Tamer; validar además seguir tras el ataque.
5. Con el registro del servidor confirmar que no aparecen técnicas habilitadas con spell ausente; no habilitar formas cuya especie/plantilla no esté activa.

## Procedencia

Los iconos son ilustraciones originales generadas para este proyecto; no contienen logos, capturas ni texturas extraídas de Digimon/WoW. Los nombres de los ataques y las ramas provienen del catálogo entregado por el usuario. No se incluyen datos DBC, MPQ, modelos ni texturas de Blizzard o de terceros.