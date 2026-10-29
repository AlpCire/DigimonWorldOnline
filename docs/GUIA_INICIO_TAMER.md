# Inicio de un Tamer nuevo

## Recorrido

La introducción es una cadena de cuatro misiones nativas y no requiere combatir:

1. **Despertar en el Mundo Digital** — acepta la misión del Guía del Mundo Digital en Villanorte y habla con la Cronista del Umbral.
2. **Recuerdos fragmentados** — habla con la Exploradora de Elwynn, en Villadorada.
3. **Una voz en Villadorada** — entrega la pista al Mensajero de Villadorada.
4. **El primer vínculo** — vuelve con el Guía. Al completarla recibes el Dispositivo Sagrado y la experiencia de misión del Tamer.

Las posiciones actuales están dentro de las zonas humanas de Villanorte y Villadorada. Los contactos tienen aspecto humano y usan plantillas propias para poder trasladarlos más adelante al Mundo Digital sin modificar NPC originales.

## Elegir el primer Digimon

Después de recibir el Dispositivo, habla con el Guía y selecciona **Elegir mi primer Digimon**. También puedes abrir `/digimon starters`. El addon muestra una tarjeta por forma con su modelo 3D y la siguiente evolución disponible.

Las líneas iniciales actualmente habilitadas son:

- **Pagumon → Impmon**, con evolución a nivel 10.
- **Patamon → Angemon**, con evolución a nivel 22.

La elección se guarda una vez por personaje. Si el personaje ya tiene un compañero, el servidor no ofrece otro inicial. Para ver la nueva interfaz, usa `/reload` o reinicia WoW después de actualizar el addon.

## Verificación técnica

El servidor valida las misiones 900701–900704, las plantillas 900163–900165 y el objeto de recompensa 900110. La migración de personajes 019 habilita Pagumon y Patamon como iniciales y activa su primera ruta. La copia previa a aplicar los cambios está guardada en `G:\World of Digimon\01_servidor\operacion\world-digimon-before-arrival-20261017.sql.gz`.
