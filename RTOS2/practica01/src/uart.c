/*
 * uart.c
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
 */

/* ---------------------------- includes --------------------------------- */





#include "sapi.h"        // <= Biblioteca sAPI

// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "uart.h"

DEBUG_PRINT_ENABLE;

/* ---------------------------- prototipos --------------------------------- */

void tareaEnviarDatosUART ( void* taskParmPtr );

/* ---------------------------- variables globales --------------------------------- */

QueueHandle_t queueUART;

/* ---------------------------- implementacion --------------------------------- */


/**
 * @fn void tareaEnviarDatosUART ( void* taskParmPtr )
 *
 * @brief inicializo colas de datos de las teclas
 *
 */

void tareaEnviarDatosUART ( void* taskParmPtr ) {

  uart_t itemColaUART;
  char cadena [25] = "\n\nParcial RTOS1\n\n";

  debugPrintConfigUart( UART_USB, 115200 );
  debugPrintlnString(cadena );

  queueUART = xQueueCreate (QUEUE_UART_L, sizeof(uart_t));

  while (TRUE) {

      xQueueReceive(queueUART, &itemColaUART, portMAX_DELAY);
      debugPrintlnString(itemColaUART.mensaje);
  }

}
