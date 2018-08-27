/* Copyright 2017-2018, Eric Pernia
 * All rights reserved.
 *
 * This file is part of sAPI Library.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*==================[inlcusiones]============================================*/

// Includes de FreeRTOS
// #include "FreeRTOSConfig.h"
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
// sAPI header
#include "sapi.h"


#include "uart.h"

#include "hm10.h"
#include "MLT-BT05.h"
/*==================[definiciones y macros]==================================*/

DEBUG_PRINT_ENABLE
CONSOLE_PRINT_ENABLE

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

DEBUG_PRINT_ENABLE;

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{

  // ---------- CONFIGURACIONES ------------------------------
  // Inicializar y configurar la plataforma

  boardConfig();

  /*
  // UART for debug messages
  debugPrintConfigUart( UART_PC, UART_PC_BAUDRATE);
  debugPrintlnString( "UART_PC configurada." );

  // Inicializar UART_232 para conectar al modulo bluetooth


  consolePrintConfigUart( UART_BLUETOOTH, UART_BLUETOOTH_BAUDRATE );
  debugPrintlnString( "UART_BLUETOOTH para modulo Bluetooth configurada." );
  if( hm10bleTest( UART_BLUETOOTH ) ){
       debugPrintlnString( "Modulo conectado correctamente." );
   }


  if(MLT_BT05_inicializar(UART_BLUETOOTH) == BPS_ERROR) {
      debugPrintlnString( "No se pudo inicializar el modulo MLT-BT05" );
      while(1);
  }
*/
/*
  consolePrintConfigUart( UART_BLUETOOTH, UART_BLUETOOTH_BAUDRATE );
  debugPrintlnString( "UART_BLUETOOTH para modulo Bluetooth configurada." );
*/

  /*
  uint8_t data = 0;

  uartWriteString( UART_PC, "Testeto si el modulo esta conectado enviando: AT\r\n" );
  if( hm10bleTest( UART_BLUETOOTH ) ){
      debugPrintlnString( "Modulo conectado correctamente." );
  }


*/

    inicializarTareaEnviarDatosUARTs();

   // Led para dar se�al de vida
   gpioWrite( LED3, ON );

   // Crear tarea en freeRTOS
   xTaskCreate(
       tareaEnviarArrayBLE,                       // Funcion de la tarea a ejecutar
      (const char *)"EnviarArrayBLE",             // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2,                 // Cantidad de stack de la tarea
      0,                                          // Parametros de tarea
      tskIDLE_PRIORITY+1,                         // Prioridad de la tarea
      0                                           // Puntero a la tarea creada en el sistema
   );

   // Crear tarea en freeRTOS
   xTaskCreate(
       tareaEnviarDatosUART,                      // Funcion de la tarea a ejecutar
      (const char *)"UART_PC",                    // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2,                 // Cantidad de stack de la tarea
      (void*) &uartPC,                             // Parametros de tarea
      tskIDLE_PRIORITY+2,                         // Prioridad de la tarea
      0                                           // Puntero a la tarea creada en el sistema
   );

   xTaskCreate(
        tareaEnviarDatosUART,                      // Funcion de la tarea a ejecutar
       (const char *)"UART_BLE",                    // Nombre de la tarea como String amigable para el usuario
       configMINIMAL_STACK_SIZE*2,                 // Cantidad de stack de la tarea
       (void*) &uartBLE,                             // Parametros de tarea
       tskIDLE_PRIORITY+2,                         // Prioridad de la tarea
       0                                           // Puntero a la tarea creada en el sistema
    );

   // Crear tarea en freeRTOS
   xTaskCreate(
       tareaRecibirStringPorTimeout,                      // Funcion de la tarea a ejecutar
      (const char *)"UART RX PC",                    // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2,                 // Cantidad de stack de la tarea
      (void*) &uartPC,                             // Parametros de tarea
      tskIDLE_PRIORITY+3,                         // Prioridad de la tarea
      0                                           // Puntero a la tarea creada en el sistema
   );

   xTaskCreate(
          tareaRecibirStringPorTimeout,                      // Funcion de la tarea a ejecutar
         (const char *)"UART RX BLE",                    // Nombre de la tarea como String amigable para el usuario
         configMINIMAL_STACK_SIZE*2,                 // Cantidad de stack de la tarea
         (void*) &uartBLE,                             // Parametros de tarea
         tskIDLE_PRIORITY+3,                         // Prioridad de la tarea
         0                                           // Puntero a la tarea creada en el sistema
      );




   // Iniciar scheduler
   vTaskStartScheduler();

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE ) {
      // Si cae en este while 1 significa que no pudo iniciar el scheduler
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

/*==================[fin del archivo]========================================*/
