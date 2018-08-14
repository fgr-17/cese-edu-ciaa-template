/*
 * uart_sw.h
 *
 *  Created on: 13 ago. 2018
 *      Author: froux
 */

#ifndef RTOS1_EJERCICIO03BIS_UART_INC_UART_SW_H_
#define RTOS1_EJERCICIO03BIS_UART_INC_UART_SW_H_


/* ------------------- defines ----------------------- */
#define BAUDRATE_MAX  500

#define BIT_START_L    ( 1 )
#define BITS_DATOS_L   ( 8 )
#define BITS_STOP_L    ( 1 )

#define BITS_SALIDA_L (BIT_START_L + BITS_DATOS_L + BITS_STOP_L)

#define UART_ESTADO_NORMAL    ( ON )
#define UART_TS               100
#define UART_PIN              GPIO2

/* ------------------- tipos ----------------------- */
typedef enum {BIT_START, BIT_D0, BIT_D1, BIT_D2, BIT_D3, BIT_D4, BIT_D5, BIT_D6, BIT_D7, BIT_STOP} bitsIndice_t;


/* ------------------- funciones ext. ----------------------- */
extern void sw_uart_sent (uint8_t byte_a_transmitir);
extern void sw_uart_config (uint16_t baudrate);

extern void tareaUART( void* taskParmPtr );

/* ------------------- variables globales ----------------------- */
extern bool_t bytePendiente;

#endif /* RTOS1_EJERCICIO03BIS_UART_INC_UART_SW_H_ */
