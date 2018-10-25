/*
 * led.h
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
 */

#ifndef RTOS1_EJERCICIO0601_INC_LED_H_
#define RTOS1_EJERCICIO0601_INC_LED_H_

/* ---------------------- tipos de datos ---------------------------------- */

typedef enum {PRIMER_FLANCO, SEGUNDO_FLANCO} estadoMedicionFlancos_t;

typedef enum {
  LEDTEC_INACTIVO,
  LEDTEC_ENCENDIDO,
  LEDTEC_APAGADO
} estadoLedTecX_t;

/* ---------------------- proto. de funciones ---------------------------------- */

extern int32_t inicializarManejadorEventoTec (void);
extern void manejadorEventoLed (Evento_t * evn);


/* ---------------------- variables globales ---------------------------------- */
extern estadoMedicionFlancos_t estadoMedicionFlancos;

#endif /* RTOS1_EJERCICIO0601_INC_LED_H_ */
