/*
 * hm10.c
 *
 *  Created on: 20 ago. 2018
 *      Author: froux
 */

/*==================[inlcusiones]============================================*/

#include <string.h>
#include <stdint.h>

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"        // <= Biblioteca sAPI

#include "queue.h"
#include "uart.h"

#include "hm10.h"


/*==================[definiciones de datos internos]=========================*/

DEBUG_PRINT_ENABLE
CONSOLE_PRINT_ENABLE


/*==================[declaraciones de funciones externas]====================*/

void tareaEnviarArrayBLE( void* taskParmPtr );


int32_t hm10bleImprimirArray (uartMap_t uart, uint16_t*array, uint32_t n);
bool_t hm10bleTest( uartMap_t uart );
void hm10blePrintATCommands( uartMap_t uart );

/*==================[variables globales]====================*/

uint16_t arrayDatos [] = {
  #include "sgn1.h"
};

uint16_t abcdario[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

#define ARRAY_DATOS_L   (sizeof(arrayDatos) / sizeof(uint16_t))
#define ABCDARIO_L    (sizeof(abcdario) / sizeof(uint16_t))

/*==================[definiciones de funciones externas]=====================*/


/**
 *  @fn void tareaEnviarArrayBLE( void* taskParmPtr )
 *
 *  @brief tarea que envia datos de un array por BLE
 */

void tareaEnviarArrayBLE( void* taskParmPtr )
{

  uartQueue_t msjuart;
  int8_t data;


  while(TRUE) {

      // Intercambia el estado del LEDB
    gpioToggle( LED3 );
    // debugPrintlnString( "Blink!" );

    strcpy(msjuart.mensaje, "blinkie!\r\n");
    xQueueSend(uartPC.queueUART, (const void * const)&msjuart, portMAX_DELAY);


    /*

    // Si leo un dato de una UART lo envio a al otra (bridge)
    if( uartReadByte( UART_PC, &data ) ) {
       uartWriteByte( UART_BLUETOOTH, data );
    }

    if( uartReadByte( UART_BLUETOOTH, &data ) ) {
       if( data == 'h' ) {
          gpioWrite( LEDB, ON );
       }
       if( data == 'l' ) {
          gpioWrite( LEDB, OFF );
       }
       uartWriteByte( UART_PC, data );
    }

    // Si presiono TEC1 imprime la lista de comandos AT
    if( !gpioRead( TEC1 ) ) {
       vTaskDelay( 500 / portTICK_RATE_MS );
       hm10blePrintATCommands( UART_BLUETOOTH );
    }

    // Si presiono TEC3 enciende el led de la pantalla de la app
    if( !gpioRead( TEC3 ) ) {
       uartWriteString( UART_BLUETOOTH, "LED ON\r\n" );
       debugPrintlnString( "LED ON\r\n");
    }
    // Si presiono TEC4 apaga el led de la pantalla de la app
    if( !gpioRead( TEC4 ) ) {
       uartWriteString( UART_BLUETOOTH, "LED OFF\r\n" );
       debugPrintlnString( "LED OFF\r\n");
    }

    // hm10bleImprimirArray(UART_BLUETOOTH, arrayDatos, ARRAY_DATOS_L);
    // debugPrintlnString( "mande array x BLE\r\n");
    // hm10bleImprimirArray(UART_BLUETOOTH, abcdario, ABCDARIO_L);
    // uartWriteString(UART_PC, "ABECEDARIO x BLE\r\n");
*/

    // Envia la tarea al estado bloqueado durante 500ms
    vTaskDelay( 1000 / portTICK_RATE_MS );
   }
}

/**
 *  @fn bool_t hm10bleTest( int32_t uart )
 *
 *  @brief envio comando de prueba y me fijo si el HM10 responde bien
 */

bool_t hm10bleTest( uartMap_t uart )
{
   uartWriteString( uart, "AT\r\n" );
   return waitForReceiveStringOrTimeoutBlocking( uart,
                                                 "OK\r\n", strlen("OK\r\n"),
                                                 50 );
}

/**
 *  @fn void hm10blePrintATCommands( int32_t uart )
 *
 *  @brief mando comando para que el HM10 envie la lista de comandos AT
 */

void hm10blePrintATCommands( uartMap_t uart )
{
   uartWriteString( uart, "AT+HELP\r\n" );
}

/**
 *  @fn int32_t hm10bleImprimirArray (uartMap_t uart) {)
 *
 *  @brief mando un array x la uart
 */

int32_t hm10bleImprimirArray (uartMap_t uart, uint16_t*array, uint32_t n) {

  uint32_t i;
  static uartWord_Byte_t envio;

  for ( i = 0; i < n; i++) {

      envio.word = array[i];

      uartWriteByte ( uart, envio.bytes[0]);
      uartWriteByte ( uart, envio.bytes[1]);
  }

  return 0;
}
