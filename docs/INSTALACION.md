# Instalación del laboratorio

Este código requiere Docker, Git y el código oficial de AzerothCore en la
revisión `01339ccfab9cd5d3d28546f79f38ad74d394b148`. La primera compilación consume
varios gigabytes y tarda bastante más que las actualizaciones del módulo.

1. Conservar una copia de seguridad del servidor y sus bases de datos. Utilizar
   una instalación de ensayo. Mantener los puertos vinculados a `127.0.0.1` hasta
   completar la configuración de acceso para amigos.
2. En la carpeta de este proyecto, ejecutar desde PowerShell
   `./scripts/Build-Module.ps1 -CorePath 'RUTA_DEL_CODIGO_AZEROTHCORE'`.
   El script verifica la revisión y copia solamente la carpeta del módulo.
   Las salidas se guardan en volúmenes Docker dedicados.
3. Con el mundo detenido, ejecutar `scripts/install.py` dentro del contenedor
   de compilación, conectado a la red del laboratorio y con PyMySQL disponible.
   Necesita `DIGIMON_DB_PASSWORD` en el entorno y permite configurar host,
   puerto, usuario y nombres de las bases. Nunca escriba la clave en el
   repositorio. Sin `--apply` solamente audita; con `--apply` instala las tablas
   y los NPC después de comprobar las colisiones.
4. Ejecutar el servidor compilado `/dist/bin/worldserver` con su configuración
   privada y sus datos locales. El volumen `world-digimon-dist` contiene el
   programa y la configuración de ejemplo del módulo en `/dist/etc/modules`.
   Copiar el `.conf.dist` del módulo a `.conf` si desea cambiar parámetros.
   No sustituir las bases ni los datos privados por archivos del repositorio.
   `docker/compose.module.yml` es un ejemplo para combinar con el Compose del
   núcleo. No es autónomo: necesita los servicios, configuración, red, datos y
   puertos privados de AzerothCore. El actualizador oficial `ac-db-import` debe
   completar las migraciones del núcleo antes del arranque; el binario de
   compilación usa el actualizador separado.
5. Revisar los registros y ejecutar las pruebas descritas en
   `PRUEBAS_MANUALES.md`. La interfaz visual requiere el cliente autorizado.

El instalador añade tres NPC/criaturas cerca del inicio humano. Los demás
inicios raciales todavía requieren ubicaciones específicas; no se consideran
validados por esta instalación.

No desinstalar eliminando las tablas: contienen la colección. Para desactivar,
detener el mundo, guardar una copia, retirar el dispositivo de las cuentas de
ensayo y cambiar `DigimonTamer.Enable`. Conservar las tablas permite investigar
y recuperar los datos posteriormente.

El proceso fija el código de AzerothCore; las dependencias de Ubuntu se obtienen
de sus repositorios. Es una receta reproducible de construcción, no una garantía
de binarios idénticos bit a bit entre fechas diferentes.

## Uso diario sin recompilar

En el laboratorio ya preparado, abrir Docker Desktop y utilizar los accesos
`INICIAR_SERVIDOR.cmd`, `VER_ESTADO.cmd` o `DETENER_SERVIDOR.cmd` de la carpeta
principal del laboratorio. No es necesario reinstalar las herramientas ni
reimportar bases cada vez. No borrar volúmenes Docker: contienen los personajes.

La prueba personal requiere un cliente autorizado build 12340, una cuenta normal
del servidor y la dirección del reino local. No hay una cuenta personal creada
con contraseña pública. Las cuentas automáticas son desechables y no deben usarse
para jugar. El acceso desde otro ordenador necesita configurar la dirección del
reino y la red; esta entrega conserva las conexiones en la misma máquina.

El envoltorio público de compilación es una receta revisada. La construcción
ejecutada en este laboratorio usa los mismos archivos de CMake y un volumen Linux
de fuentes para acelerar las actualizaciones desde Windows. Los resultados de
esa compilación y sus pruebas se describen en `PRUEBAS.md`; no se afirma haber
reinstalado todo el laboratorio desde cero con el envoltorio público.
