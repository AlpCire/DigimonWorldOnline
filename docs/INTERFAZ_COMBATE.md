# Dispositivo y modo Luchar — Alpha 0.3.2

## Dentro del juego

1. Abre el cliente después de instalar el complemento WorldDigimon. En la selección
   de personajes, revisa «Accesorios» si no aparece el botón **Dispositivo** al entrar.
2. Pulsa **Dispositivo** en la parte superior de la pantalla y elige **Luchar**.
   También puedes abrir el objeto desde la mochila o escribir `/digimon`.
3. Selecciona un enemigo y pulsa **Atacar**. Cuando el compañero esté cerca,
   pulsa el icono de la técnica que quieras ejecutar. Puedes seguir moviendo al Tamer.
4. Para recuperar la barra anterior, abre Dispositivo y elige **Salir de Luchar**.

La barra ocupa la posición de la barra principal y utiliza sus teclas asignadas.
Los controles propios tienen una capa explícita por encima de la interfaz y
ocultan visualmente los doce botones originales mientras Luchar está activo.
Al salir se recupera la transparencia anterior de cada botón. Esta corrección
evita que los iconos originales tapen las técnicas del compañero.
Con los controles predeterminados:

| Posición | Acción |
|---|---|
| 1 | Ordenar ataque al objetivo seleccionado |
| 2 | Dejar de atacar y seguir al dueño |
| 3–7 | Técnicas equipadas 1–5; en Rookie solo hay dos espacios utilizables |
| 8 | Intentar capturar el salvaje seleccionado |
| 9 | Invocar compañero |
| 10 | Retirar compañero fuera de combate |
| 11 | Descansar y recuperar fuera de combate |
| 12 | Abrir el dispositivo |

Las técnicas vacías aparecen atenuadas. La barra muestra nivel, salud y energía;
el sombreado de recuperación indica cuándo vuelve a estar disponible una técnica.
Las habilidades aprendidas deben estar equipadas para aparecer aquí.

**Mordisco Digital** necesita un enemigo al alcance del compañero y consume 25
de energía según los datos actuales del laboratorio. Si el lobo está lejos,
se le ordena acercarse y se indica que vuelvas a pulsar la técnica al llegar.
**Aullido de Guerra** es una mejora temporal: consume 20 de energía y tiene
40 segundos de recuperación. Su resultado se comprueba por el beneficio aplicado,
no por un golpe al enemigo. La energía se recupera a razón de 24 cada cuatro segundos
con la tasa de foco del laboratorio en 1.

Cambiar entre las barras durante un combate queda pendiente hasta salir de él.
Las órdenes de una barra ya activa funcionan durante el combate. Se conserva el
modo elegido al reconectar. No se reescriben los huecos, macros ni teclas guardados
del personaje; los controles originales vuelven al salir de Luchar.

## Instalación y actualización

Con WoW completamente cerrado, ejecutar desde el repositorio:

```powershell
./scripts/Install-ClientAddon.ps1 -ClientPath 'RUTA_DEL_CLIENTE' -BackupPath 'RUTA_DE_COPIAS_PRIVADAS'
```

El instalador copia únicamente `client/WorldDigimon` a `Interface/AddOns/WorldDigimon`.
Guarda una copia de los archivos anteriores y de `Cache/WDB/*/itemcache.wdb` antes
de retirar solo esa caché de objetos, para que el cliente vuelva a consultar el
Dispositivo Sagrado actualizado. No elimina otras cachés ni modifica ejecutables.
Hay que arrancar de nuevo el cliente: una carpeta de complemento nueva necesita
un inicio completo para incorporarse a su lista.

Actualizar también el módulo del servidor y ejecutar su instalador con el mundo
detenido. La actualización cambia solo el hechizo heredado 8690 del objeto 900110
identificado como DigimonDevice a un uso instantáneo 18282, sin modificar la Piedra
de hogar 6948 ni sobrescribir un hechizo personalizado por el administrador.
El script intercepta el uso y confirma al cliente que puede volver a utilizar el
objeto. La presentación del complemento da al objeto un icono y descripción propios.

## Implementación y alcance

- El complemento envía órdenes explícitas. El servidor comprueba propiedad, vida,
  técnica aprendida/equipada, etapa, objetivo, distancia, energía y recuperación.
- No se enseñan los ataques al humano ni se habilita daño humano. Sigue activo
  un solo compañero con IA propia, sin depender de una mascota nativa de cazador.
- El servidor usa el gestor de recuperación global del núcleo y registra la
  recuperación específica de cada técnica. La regeneración se programa en la IA.
- Mensajes privados de estado `WDMMO`, versión V1: estado del actor y cinco espacios
  de equipo, sin consultas SQL por segundo. El cliente solo acepta mensajes del
  propio personaje y no cambia atributos protegidos durante el combate.
- La compra y gestión de la colección conservan los menús del servidor. La barra
  utiliza la interfaz principal original; otros complementos que la sustituyan
  requieren comprobar compatibilidad.
- Los iconos son referencias a recursos ya presentes en el cliente. El repositorio
  no incluye texturas, DBC, MPQ ni otros archivos del juego.

Las comprobaciones Lua fuera del juego no certifican el sistema protegido real del
cliente. Tras instalar, comprobar visualmente movimiento, teclas, cambio de barras,
técnicas y ausencia de errores Lua. Los resultados de protocolo se registran por
separado en PRUEBAS.md.
