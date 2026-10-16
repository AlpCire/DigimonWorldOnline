# Alcance de la Alpha de laboratorio

- Falta la validación visual con dos clientes WoW 3.3.5a autorizados. No se ha
  descargado ni redistribuido un cliente. Los puertos permanecen locales.
- La acogida se sitúa en Northshire y se prueba con humanos. Otras razas, todas
  las clases, grupos, monturas, vehículos, muerte/resurrección e instancias
  requieren ampliar la regresión antes de abrir acceso a amigos.
- El compañero usa IA propia y la barra del complemento WorldDigimon, sin barra nativa de mascota. Las clases
  originales siguen existiendo; los filtros del servidor restringen su ofensiva.
  No se ha certificado cada objeto, proc o habilidad residual del cliente.
- Un actor operativo por Tamer, una posición de equipo utilizable, colección
  inicial de 30. Los desbloqueos de nivel son planificación configurable,
  no acceso a dos o tres combatientes en esta Alpha.
- Oso, felino y rapaz están catalogados y tienen plantillas locales verificadas,
  pero no se ofrecen como iniciales ni capturas. El motor de evolución por especie
  y nivel está implementado; el ejemplo Patamon/Angemon sigue desactivado hasta
  instalar sus modelos. DigiPociones y técnicas superiores siguen desactivadas.
- Las técnicas dependen de especie, etapa, moneda y entrenador; no hay árbol de
  requisitos ni balance definitivo. La curva del compañero
  es propia: `100 + 75 × nivel`, máximo 80; no reproduce la curva de WoW.
- Descansar recupera gratis al compañero fuera de combate. Es una ayuda de ensayo,
  pendiente de sustituir por las mecánicas de cuidados y costes del diseño.
- Las escrituras usan una conexión síncrona y un bloqueo. No se hace SQL por tick,
  pero hay que medir latencia y concurrencia antes de escalar el servidor.
- Capturas admitidas solo en criaturas persistentes, habilitadas expresamente y
  debilitadas por el actor activo. El bloqueo usa el primer dueño del botín.
  No hay captura compartida en grupo ni capturas entre instancias de una misma vida.
- La vida de cada salvaje queda registrada por aparición, mapa e instancia.
  Una captura confirmada se conserva aunque el proceso termine antes de retirar
  la criatura. La prueba de reinicio acredita datos guardados, no una simulación
  de todos los posibles cortes eléctricos o fallos de disco.
- El instalador se ejecuta fuera del proceso con el mundo detenido. La tabla de
  versiones evita repetir migraciones completas. DDL en MySQL hace confirmaciones
  implícitas: una interrupción a mitad de un `ALTER TABLE` necesita inspección y
  recuperación de la copia; no se promete atomicidad del despliegue completo.
- El dispositivo actualizado utiliza un uso instantáneo interceptado. Para sustituir
  su hechizo anterior en el cliente hay que renovar su caché de objetos con WoW cerrado;
  el instalador del complemento conserva una copia. El control de combate requiere
  WorldDigimon; los menús del servidor siguen disponibles con `.digimon`.
- La barra del complemento está diseñada para la interfaz principal original. Los
  cambios de modo en combate esperan hasta salir de él; no se ha certificado cada
  combinación con complementos que sustituyan las barras, vehículos o formas.
- La configuración de IDs debe concordar con instalador y catálogo. Cambiar solo
  el archivo `.conf` no migra objetos, NPC o criaturas existentes.
- El ensayo de seguidores no demuestra que WoW admita tres mascotas nativas:
  son invocaciones pasivas durante 30 segundos, sin control de combate, progreso
  ni escritura en colección. Solo un administrador puede activarlo y se entrega apagado.
- Alpha 0.2 ya se ha auditado y comparado; no se ha instalado su módulo ni se
  ha certificado dentro del juego. Sigue pendiente el kit de preparación de
  modelos y no hay recursos Digimon de terceros certificados.

Antes de actualizar: copia de seguridad, pruebas sobre una copia de las bases,
revisión de colisiones de IDs y verificación de los cambios propuestos. El módulo
no se declara compatible con otros módulos que reemplacen recompensas, daño,
Gossip, control de mascotas o tasas de experiencia sin una prueba conjunta.
