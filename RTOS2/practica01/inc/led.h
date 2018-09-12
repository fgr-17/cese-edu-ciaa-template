/*
 * led.h
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
 */

#ifndef RTOS1_EJERCICIO0601_INC_LED_H_
#define RTOS1_EJERCICIO0601_INC_LED_H_


/* ---------------------- definicion de ctes ---------------------------------- */
#define TIEMPO_PARPADEO     500

/* ---------------------- tipos de datos ---------------------------------- */

typedef enum {PRIMER_FLANCO, SEGUNDO_FLANCO} estadoMedicionFlancos_t;

/* ---------------------- proto. de funciones ---------------------------------- */

extern void tareaMedicionFlancos( void* taskParmPtr );
extern void tareaParpadearLedG( void* taskParmPtr );
extern void tareaParpadearLed1( void* taskParmPtr );
extern void tareaParpadearLed2( void* taskParmPtr );
extern void tareaActualizarPeriodoLeds( void* taskParmPtr );
void tareaMedirTecla (void* taskParamPtr);

/* ---------------------- variables globales ---------------------------------- */
extern estadoMedicionFlancos_t estadoMedicionFlancos;

#endif /* RTOS1_EJERCICIO0601_INC_LED_H_ */
