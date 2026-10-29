# Captura y Digialmacén (primer MVP)

El Digiequipo admite seis Digimon y el Tamer sigue invocando uno a la vez en esta Alpha. Cada Tamer comienza con dos cajas de almacenamiento de 15 espacios. Los Digimon capturados entran en el primer hueco del equipo y, al llenarlo, pasan al Digialmacén. El addon permite moverlos entre ambos espacios; solo puede invocarse un miembro del equipo.

La captura se inicia con el Digivice cuando el salvaje autorizado está debilitado por el compañero. El Tamer debe pulsar cinco nodos visibles en un máximo de ocho segundos. El servidor valida el objetivo, la propiedad del intento, cada nodo único, el plazo y el espacio disponible antes de guardar la captura. Fallar consume el tiempo de recuperación del dispositivo.

Las cajas adicionales cuestan oro creciente y un Núcleo de expansión. La tercera caja cuesta 1 oro, la cuarta 4, la quinta 9, y así sucesivamente; cada una añade 15 espacios, hasta un máximo de 100 cajas. Los Núcleos de expansión se obtienen como botín de los Pagumon de prueba.

## Prueba e instalación

1. Compilar `modules/mod-digimon-tamer` en el entorno de AzerothCore para verificar compatibilidad con la rama instalada.
2. Ejecutar `scripts/install.py` en modo de auditoría y luego con `--apply` contra las bases de datos de laboratorio. La migración 022 crea el Digiequipo y conserva las colecciones actuales; la migración de mundo 005 configura el botín del núcleo.
3. Instalar el contenido de `client/WorldDigimon` en `Interface/AddOns/WorldDigimon`, iniciar sesión y usar el Dispositivo.
4. Capturar un Pagumon de prueba, comprobar que ocupa una plaza de equipo o una caja, moverlo a ambos lados y reiniciar el servidor para confirmar persistencia.
5. Llenar las seis plazas; comprobar que la captura siguiente va a la Caja 1. Intentar capturar con caja llena y verificar el mensaje de capacidad. Comprar una caja con oro y un Núcleo, volver a capturar y confirmar el espacio nuevo.

La integración conserva la propiedad y persistencia en la base Characters. La interfaz de clics depende del addon, por lo que un cliente modificado podría automatizar las pulsaciones; el servidor mantiene la validación de estado, tiempo, duplicados, objetivo y capacidad.
