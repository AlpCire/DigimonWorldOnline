# Prueba con dos clientes autorizados

Usar un laboratorio privado, con copia de las bases de datos. Necesita un
cliente WoW 3.3.5a, build 12340, obtenido y utilizado con autorización. Este
repositorio no lo proporciona. Los pasos siguientes son un protocolo pendiente,
no una declaración de pruebas superadas.

1. Iniciar autenticación y mundo. Revisar el registro `module.digimon` y confirmar
   que no hay errores SQL ni referencias a plantillas ausentes.
2. Crear dos cuentas normales, sin privilegios GM. En cada una crear un humano,
   uno guerrero y otro mago. Entrar simultáneamente en Northshire.
3. Hablar con el Guía del Mundo Digital y elegir Lobo digital. Confirmar que cada
   lobo sigue únicamente a su dueño y que se entrega el Dispositivo Sagrado.
4. Volver al guía, repetir invocación diez veces y reconectar. Confirmar una sola
   entrada en colección y un único compañero por jugador.
5. Seleccionar un enemigo normal, ordenar el ataque con el dispositivo y
   comprobar movimiento, alcance, amenazas y persecución. Probar la orden de
   seguir. El lobo no debe atacar a jugadores o criaturas amistosas.
6. Intentar atacar cuerpo a cuerpo y lanzar Bola de Fuego con el humano. Debe
   infligir cero daño y rechazar el hechizo ofensivo. Verificar movimiento,
   interacción, sanación permitida, montura y recogida de botín.
7. Aceptar una misión de matar enemigos normales. Dejar que el compañero mate
   uno y comprobar experiencia, objetivo, reputación y acceso al botín. Repetir
   en grupo para detectar atribución incorrecta.
8. Dejar herido al compañero, cerrar sesión y volver. Comprobar salud y
   propietario. Retirarlo, reconectar y confirmar que permanece retirado.
9. Cambiar de mapa, morir, resucitar y probar desconexión inesperada. Confirmar
   que no quedan compañeros huérfanos ni actores duplicados.
10. Guardar datos y reiniciar solamente el mundo. Reconectar ambas cuentas y
    comparar compañero, salud, nivel, experiencia y técnicas con los valores
    previos. No modificar las tablas manualmente para conseguir el resultado.

11. Visitar al entrenador. Con 100 DigiBits, aprender Mordisco Digital por 20 y
    Aullido de Guerra por 35. Comprobar saldos 80 y 45, equipar ambas por separado,
    utilizar las técnicas y reconectar. Las tres técnicas superiores no se ofrecen.
12. Intentar comprar desde el dispositivo y a distancia del entrenador. Intentar
    comprar otra vez una técnica conocida y comprar sin saldo. No debe concederse
    ni cobrarse una compra inválida. Deshacer equipo no debe borrar el aprendizaje.
13. Debilitar con el compañero un Lobo de datos salvaje a un máximo del 25 % de
    salud; detener sus ataques y capturarlo desde el dispositivo. La probabilidad
    normal es 35 % y la recuperación de cada intento válido, 10 segundos.
    Una captura confirmada añade una entrada y retira el salvaje; conserva nivel y salud.
14. Probar salud completa, jefe, criatura de misión, mascota ajena, distancia,
    falta de línea de visión y objetivo debilitado por el otro Tamer. Deben
    rechazarse. Repetir clics y reconectar durante la recuperación no debe duplicar.
15. Abrir la colección, activar al capturado fuera de combate y comprobar un
    único actor. Volver al inicial, retirar, invocar y reconectar. Conservar
    técnicas por compañero y salud; cambiar durante combate debe rechazarse.
16. Usar el **objeto real** desde las bolsas, además de `.digimon`; probar bolsas
    llenas, dispositivo en banco, menús cancelados y elección de otro objetivo
    mientras el menú está abierto. Comprobar que no se activa la piedra de hogar.
17. Verificar límites: solo Lobo digital seleccionable, dos técnicas Rookie y un
    compañero operativo. No hay evoluciones, pociones ni segundo combatiente
    disponibles. Los espacios futuros se muestran como planificación.
18. Verificar una misión que entrega una cantidad conocida de XP. Comparar la
    recompensa humana con x0,70 y comprobar que el progreso del compañero no
    cambia al entregar la misión. Probar después una muerte en grupo y botín/reputación.
19. Repetir el reinicio con un compañero capturado activo y técnicas aprendidas
    en otro compañero. Comparar toda la colección y las posiciones del equipo.
20. Repetir muerte, mapa distinto y desconexión inesperada con poca salud, con
    menú de compra abierto y después de capturar. Conservar datos y evitar actores huérfanos.

El ensayo de tres seguidores se reserva a un laboratorio separado. Activarlo
exige administrador y configuración explícita; no es una función jugable para
cuentas normales y debe quedar desactivado al terminar.

Registrar fecha, revisión del núcleo y módulo, cuentas de ensayo, resultado,
registro del servidor y captura de pantalla de los fallos. No adjuntar claves ni
archivos del cliente al informe público.
