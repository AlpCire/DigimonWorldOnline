# Empatía del Digimon

Empatía es el vínculo persistente entre un Tamer y cada compañero de su colección.
Es independiente de las afinidades elementales usadas por las pruebas evolutivas,
se guarda entre sesiones, tiene escala de 0 a 100 y nunca disminuye.

## Fuentes de progreso en esta primera versión

- +1 por victoria válida del compañero contra una criatura que aún concede XP.
- +1 por derrota del objetivo exacto de un encargo evolutivo, aunque ese objetivo
  ya no conceda XP por nivel.
- +5 al entregar una misión evolutiva nativa, para el compañero ligado a ella
  cuando se aceptó.
- +1 al descansar y recuperar al compañero, como máximo una vez cada 24 horas
  por compañero.

Las victorias se validan con las reglas actuales de recompensa (dueño del botín,
compañero gestionado por el módulo y objetivo válido). Los puntos se limitan a 100.
No se aplican penalizaciones por inactividad ni se eligen todavía umbrales de
Empatía para digievoluciones; esos requisitos deben diseñarse por rama.

El Digivice muestra el valor y un rango descriptivo: Primer contacto, Confianza,
Compañerismo, Armonía, Vínculo profundo o Vínculo absoluto. Son etiquetas de
interfaz, no niveles ni bonificaciones de combate.

La migración de personajes `018_digimon_empathy.sql` crea la persistencia sin
alterar las filas actuales de colección. Los valores iniciales son cero.
