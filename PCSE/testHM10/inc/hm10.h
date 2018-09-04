/*
 * hm10.h
 *
 *  Created on: 20 ago. 2018
 *      Author: froux
 */

#ifndef PCSE_TESTHM10_INC_HM10_H_
#define PCSE_TESTHM10_INC_HM10_H_

/*==================[definiciones y macros]==================================*/

#define UART_PC        UART_USB
#define UART_BLE      UART_232

#define UART_PC_BAUDRATE            115200
#define UART_BLE_BAUDRATE         9600

/*==================[tipos de dato]====================*/




/*==================[declaraciones de funciones externas]====================*/

// extern void tareaEnviarArrayBLE( void* taskParmPtr );

extern bool_t hm10bleTest( uartMap_t uart );
extern void hm10blePrintATCommands( uartMap_t uart );
extern int32_t hm10bleImprimirArray (uartMap_t uart, uint16_t*array, uint32_t n);

#endif /* PCSE_TESTHM10_INC_HM10_H_ */
