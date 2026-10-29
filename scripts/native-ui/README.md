# Herramientas privadas de migración UI
**Ensayo pausado el 29/10/2026:** la copia aislada presentó marcos superpuestos y contenido desactualizado. Estas herramientas son material de investigación; no producen una interfaz aprobada para jugadores.

Estas herramientas producen parches de **ensayo local** para el cliente WoW 3.3.5a esES compilación 12340. No publican los modelos ni las texturas. La migración no está lista para distribución.

1. Instalar la dependencia indicada en `package.json` en esta carpeta.
2. Crear una copia del ejecutable con `Build-FrameXMLProbe.ps1` en una ruta privada y distinta de `Wow.exe`; el script verifica el hash y los bytes originales.
3. Ejecutar `node build-framexml-probe.cjs <ruta-Data-del-cliente> <salida-privada-probe.MPQ>`.
4. Preparar una copia completa del cliente con `Prepare-IsolatedClient.ps1` e insertar en ella el MPQ de ensayo.
5. Comprobar dentro del juego que aparece en el chat `WORLD DIGIMON: interfaz integrada de prueba cargada.`
6. Solo entonces, ejecutar `node build-nativeui-package.cjs <ruta-Data-del-cliente> <ruta-WorldDigimon-fuente> <salida-privada-native.MPQ>`, cambiar el MPQ de la copia con `Switch-IsolatedPatch.ps1` y repetir las pruebas de interfaz. Este script guarda una copia del parche anterior y se niega a actuar sobre la instalación principal.

El empaquetador añade los diez módulos Lua que figuran en `WorldDigimon.toc` y los recursos `.blp`/`.tga` bajo `Interface\FrameXML\WorldDigimonNative`. El fichero `FrameXML.toc` base se acepta solo si su SHA-256 coincide con el auditado. Si cambia la versión del cliente, los scripts se detienen.

El candidato más reciente añade `## SavedVariablesPerCharacter: WorldDigimonDB` a `FrameXML.toc` para comprobar si el cliente admite persistencia en esta vía. **Es una hipótesis sin validar en partida.** Si WoW ignora esa directiva en FrameXML, la configuración y el mapa deberán guardarse en el servidor antes de sustituir el addon en producción. No publicar parches que contengan arte de terceros sin licencia documentada.
