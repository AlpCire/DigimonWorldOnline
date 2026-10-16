# Próximos pasos para cerrar la Alpha 0.3

## 1. Supervisión dentro del juego

Conectar dos clientes autorizados, recorrer `PRUEBAS_MANUALES.md` y registrar
problemas de interfaz, seguimiento, combate y guardado. Mantener un compañero
operativo hasta cerrar esos fallos. Decidir después si se permite acceso privado
desde otras máquinas; requiere revisar cuentas, red y configuración del reino.

## 2. Incorporar el material original

Recibir el ZIP Alpha 0.2, inventariarlo sin ejecutar su contenido y comparar
README, TASK_FOR_CODEX, código, SQL y configuración con esta implementación.
No sustituir bases ni archivos propios por los del ZIP sin revisar diferencias.
Recibir también el kit de modelos; registrar procedencia, hashes y permisos.

## 3. Primer modelo propio o autorizado

Validar geometría, rig, texturas y animaciones; exportar M2/SKIN para build 12340;
probar escala, selección, cámara, ataque, muerte y seguimiento en el cliente.
El cubo sintético del laboratorio solo verifica parte del formato. No equivale
a una criatura terminada ni acredita derechos de distribución de un Digimon.

## 4. Progresión y cuidados

Decidir curva de XP, economía DigiBits, especies compatibles con cada técnica,
requisitos, DigiPociones y coste de recuperación. Las evoluciones deberán cambiar
la etapa y los espacios de forma transaccional, conservando las técnicas aprendidas
y exigiendo entrenamiento para las nuevas. Las filas de evolución actuales están
desactivadas y son valores de ejemplo pendientes de diseño.

## 5. Equipo y apoyo

La colección almacena, `digimon_team` prepara y `digimon_active` identifica al
principal. Habilitar primero la preparación de posiciones adicionales según
`digimon_unlocks`, con cambio solo fuera de combate y validación del servidor.
Después ampliar el ensayo a un principal y una IA de apoyo, con órdenes limitadas,
atribución de recompensas, formación, teletransporte y limpieza al desconectar.
Solo habilitar dos o tres combatientes tras pruebas de concurrencia y balance.

## 6. Mundo Digital

Preparar una zona pequeña editada con origen de recursos documentado: entrada,
guía, entrenador, captura y misión de recorrido. Validar mapas, colisiones y
navegación antes de ampliar zonas. No distribuir paquetes del cliente original.
Los mapas, la historia y el contenido completo del MMO quedan fuera de este
primer módulo jugable.
