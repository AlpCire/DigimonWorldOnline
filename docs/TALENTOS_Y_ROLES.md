# Talentos y especializaciones por forma

Cada Digimon recibe un punto por nivel a partir del nivel 2. Su nivel y progreso son
únicos, pero cada forma evolutiva guarda sus propios puntos de talento y su barra de
técnicas. Al evolucionar, el servidor carga sólo la configuración de la forma activa.
Al volver a una forma anterior, recupera sus puntos y técnicas. Ninguna bonificación
de talentos de otra forma se suma al daño, a la curación ni a los atributos actuales.

La interfaz del addon presenta 27 talentos en tres ramas: Asalto, Guardia y Soporte.
Los nodos requieren puntos en su rama y algunos exigen un talento anterior. El
servidor comprueba los requisitos y calcula los efectos; la interfaz informa de
los rangos y beneficios al pasar el ratón. Una forma puede invertir en cualquiera
de las tres ramas, aunque su función principal dependa de la especie.

Cada rama ofrece 48 rangos, para un total de 144. Un Digimon de nivel 100
dispone de 99 puntos: ya no puede completar las tres ramas a la vez. Los nueve
talentos de progresión ampliada tienen diez rangos: Furia de datos, Filo veloz,
Explotar debilidad, Núcleo blindado, Capa protectora, Reserva defensiva,
Restauración, Reserva espiritual y Auxilio. Los costes por punto no cambian.
Los talentos de firma de Angemon convierten el daño de Furia y Explotar
debilidad en curación. El rango 5 de Restauración sigue desbloqueando Pulso
sanador, aunque ahora puede seguir mejorándose hasta el rango 10.

## Funciones existentes

- Angemon es Sanador. Patamon se mantiene como forma inicial; sus ramas futuras no
  están definidas aquí y podrán tener otras funciones.
- Impmon y las formas actualmente implementadas de sus cuatro rutas son DPS.
  Comparten la estructura de tres ramas, pero dos talentos de firma cambian
  según la ruta: Impmon, hechicería, oscuridad, espectro, brujería o artillería.
- Garurumon y las formas sin una especialización propia usan el perfil básico.

La función no se elige manualmente. El servidor la carga de
`digimon_form_specializations` al cambiar de forma. Los perfiles de las rutas que
faltan podrán añadirse en una migración posterior sin alterar las configuraciones
existentes. Para Angemon, los talentos de firma aumentan curación, no daño.

## Migración y reversión

La migración 025 asigna los puntos y las técnicas equipadas existentes a la forma
que el Digimon tenía al actualizar el servidor. No crea puntos en ninguna otra
forma. Conserva las técnicas aprendidas a nivel de Digimon, pero sólo deja equipar
y usar las autorizadas por la especie y sus talentos activos. La reasignación de
puntos limpia únicamente la forma activa.

Antes de aplicar 025 hay que guardar una copia de la base `acore_characters`.
La migración cambia claves de tablas y no debe revertirse borrando columnas en
producción; ante un fallo se restaura la copia y el binario anterior.

## Pruebas en juego

1. Dar cuatro rangos a Furia de datos en Impmon y medir daño con el mismo enemigo.
2. Evolucionar a Devimon: debe mostrar puntos propios sin sumar el 8 % de Impmon.
   Invertir cuatro rangos en Poder oscuro y medir de nuevo. Debe aplicar 8 %, no 16 %.
3. Volver a Impmon: deben reaparecer sus cuatro rangos y sus técnicas equipadas.
   Repetir tras desconectar y reconectar.
4. Reasignar talentos de Devimon: Impmon debe conservar sus puntos.
5. Evolucionar Patamon a Angemon: debe mostrar Sanador y los talentos de curación.
   Al volver a Patamon, debe recuperar su configuración de forma inicial.
6. Comprobar que una técnica desbloqueada por talento no se puede usar después de
   reasignar o si la forma activa no cumple el requisito. Tampoco debe figurar
   como equipable en el entrenador tras reasignar.
7. Subir de 5/10 a 6/10 en uno de los talentos ampliados y verificar que el
   beneficio y el número de puntos libres cambian en el addon y tras reconectar.
8. A nivel 100, comprobar que quedan 99 puntos invertibles y que al menos
   45 rangos del conjunto completo permanecen sin invertir.

## Balance de rangos

Surcador del viento concede un 2 % de velocidad de movimiento al Tamer por
rango invertido, hasta un 10 % con 5/5. Sólo se aplica mientras el Digimon está
invocado. La interfaz indica el beneficio actual y el del siguiente rango.

Fluidez reduce un 3 % por rango la reutilización de Pulso sanador y Sanación
de emergencia. Eco protector sólo genera su barrera al curar a un Digimon
aliado que no tenga ya esa barrera del mismo sanador; los textos de ayuda del
addon describen ambas limitaciones.

El árbol nativo de WoW 3.3.5a está ligado a clases y mascotas del cliente. Por eso
esta interfaz usa el addon, con validación y efectos gestionados por AzerothCore.

## Objetivos de evolución del Bosque Esmeralda

Las cuatro primeras rutas de Impmon usan ahora las criaturas ya presentes en el
Bosque Esmeralda: Hechicería y Espectro piden ocho Pagumon desfasados (900171);
Oscuridad y Brujería piden el Eco sombrío de Devimon (900174) una vez. Los
requisitos internos del compañero y los objetivos de las misiones nativas
coinciden. La cadena Pagumon a Impmon y los encargos humanos que pedían
Pagumon nodal también apuntan a los enemigos del bosque. Los ensayos ya
completados conservan sus recompensas; una misión en curso puede necesitar
abandonarse y aceptarse de nuevo para actualizar su texto en el cliente.
