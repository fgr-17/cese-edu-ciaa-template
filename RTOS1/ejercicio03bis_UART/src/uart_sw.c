/*
 * uart_sw.c
 *
 *  Created on: 13 ago. 2018
 *      Author: froux
 */

/* ------------------- includes ----------------------- */
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

#include "uart_sw.h"

DEBUG_PRINT_ENABLE;
/* ------------------- prototipos ----------------------- */
void sw_uart_sent (uint8_t byte_a_transmitir);
void sw_uart_config (uint16_t baudrate);



/* ------------------- variables globales ----------------------- */

bool_t bitsSalida[BITS_SALIDA_L];

bool_t bytePendiente = FALSE;

TickType_t tBaudrate = 0;

/* ------------------- funciones ----------------------- */

/**
 * @fn void sw_uart_sent (uint8_t byte_a_transmitir)
 * @brief envio un byte por uart gpio
 * @author froux
 */
void sw_uart_sent (uint8_t byte_a_transmitir) {

  bitsIndice_t i;

  int8_t mascaraBits = 0x01;

  bitsSalida[BIT_START] = OFF;
  bitsSalida[BIT_STOP] = ON;

  for (i = BIT_D0; i < BIT_D7; i++) {

      if(byte_a_transmitir & mascaraBits)
        bitsSalida [i] = ON;
      else
        bitsSalida [i] = OFF;

      mascaraBits <<= 1;

  }

  bytePendiente = TRUE;


}

/**
 * @fn void sw_uart_config (uint16_t baudrate)
 * @brief inicializo el servicio de salida UART por soft
 * @author froux
 */

void sw_uart_config (uint16_t baudrate) {

  debugPrintConfigUart( UART_USB, 115200 );
  debugPrintlnString( "Blinky con freeRTOS y sAPI." );

  gpioInit( UART_PIN, GPIO_OUTPUT );
  gpioWrite(UART_PIN, ON);


  if  (baudrate > BAUDRATE_MAX) {

      return;
  }

  tBaudrate = 1000 / baudrate;
  return;

}

/**
 * @fn void tareaUART ( void* taskParmPtr )
 * @brief tarea que tira los bits afuera si hay algo para enviar
 * @author froux
 */

void tareaUART( void* taskParmPtr ) {

  bitsIndice_t i;
  TickType_t tiempoInicio;

  gpioWrite(UART_PIN, ON);
  while(TRUE) {

      if(bytePendiente) {

          bytePendiente = FALSE;

          for (i = BIT_START; i < BIT_STOP; i++) {
              tiempoInicio = xTaskGetTickCount();
              gpioWrite(UART_PIN, bitsSalida[i]);
              vTaskDelayUntil(&tiempoInicio, tBaudrate / portTICK_RATE_MS);

          }
          gpioWrite(UART_PIN, ON);
          bytePendiente = FALSE;
      }


      vTaskDelay(UART_TS / portTICK_RATE_MS);



  }




}

