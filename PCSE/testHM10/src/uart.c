/*
 * uart.c
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
 */

/* ---------------------------- includes --------------------------------- */


#include "stdint.h"
#include "sapi.h"        // <= Biblioteca sAPI

// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include "hm10.h"
#include "uart.h"

/* ---------------------------- prototipos --------------------------------- */
int32_t inicializarStructUart (uart_t*uart, uartMap_t perif, uint32_t baudrate);
int32_t inicializarTareaEnviarDatosUARTs ( void );
void tareaEnviarDatosUART ( void* taskParmPtr );

/* ---------------------------- variables globales --------------------------------- */

uart_t uartPC;
uart_t uartBLE;

/* ---------------------------- implementacion --------------------------------- */


/**
 * @fn int32_t inicializarTareaEnviarDatosUART ( void ) {
 *
 * @brief inicializo colas de datos de las teclas
 *
 */

int32_t inicializarStructUart (uart_t*uart, uartMap_t perif, uint32_t baudrate) {

  uart->perif = perif;
  uart->baudrate = baudrate;
  uart->queueUART = xQueueCreate (QUEUE_UART_L, sizeof(uartQueue_t));
  return 0;
}

/**
 * @fn int32_t inicializarTareaEnviarDatosUART ( void ) {
 *
 * @brief inicializo colas de datos de las teclas
 */

int32_t inicializarTareaEnviarDatosUARTs ( void ) {

  inicializarStructUart(&uartPC, UART_PC, UART_PC_BAUDRATE);
  uartRxInterruptSet(UART_PC, TRUE);
  uartInit( uartPC.perif, uartPC.baudrate);

  inicializarStructUart(&uartBLE, UART_BLE, UART_BLE_BAUDRATE);
  uartInit( uartBLE.perif, uartBLE.baudrate);


  return 0;
}


/**
 * @fn void tareaEnviarDatosUART ( void* taskParmPtr )
 *
 * @brief inicializo colas de datos de las teclas
 *
 */

void tareaEnviarDatosUART ( void* uartN ) {

  uartQueue_t itemColaUART;
  uart_t* uartCast;
  char cadena [25] = "\n\nInicializacion UART\n\n";

  uartCast = uartN;
  // queueUART = xQueueCreate (QUEUE_UART_L, sizeof(uart_t));

  uartWriteString (uartCast->perif, cadena);

  while (TRUE) {

      xQueueReceive(uartCast->queueUART, &itemColaUART, portMAX_DELAY);
      uartWriteString(uartCast->perif, itemColaUART.mensaje);

      // hacerlo con enviar byte? Chip_UART_SendByte(lpcUarts

  }

}


/**
 * @fn void UART3_IRQHandler(void)
 *
 * @brief handler de la uart PC
 *
 */

void UART5_IRQHandler(void) {

  static int a;

  a = 0;

  return;



}

/**
 * @fn void UART5_IRQHandler(void)
 *
 * @brief handler de la uart BLE
 *
 */
void UART3_IRQHandler(void) {



  static int a;

  a = 0;

  return;



}


void UART2_IRQHandler(void) {



  static int a;

  a = 0;

  return;



}
