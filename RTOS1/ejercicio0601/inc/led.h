/*
 * led.h
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
 */

#ifndef RTOS1_EJERCICIO0601_INC_LED_H_
#define RTOS1_EJERCICIO0601_INC_LED_H_


extern void tareaParpadearLedG( void* taskParmPtr );
extern void tareaParpadearLed1( void* taskParmPtr );
extern void tareaActualizarPeriodoLeds( void* taskParmPtr );

#endif /* RTOS1_EJERCICIO0601_INC_LED_H_ */
