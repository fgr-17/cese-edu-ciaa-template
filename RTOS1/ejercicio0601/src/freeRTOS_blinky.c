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
// #include "../../ejercicio0601/inc/FreeRTOSConfig.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// sAPI header
#include "sapi.h"

#include "antirreb.h"
#include "gpio.h"


/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

DEBUG_PRINT_ENABLE;


/* -------------------- defines ------------------------------ */

#define TON_MIN         100
#define TON_MAX         900

#define TON_INICIAL     0
#define TON_INCREMENTO  100

#define PERIODO         1000

/* -------------------- funciones ------------------------------ */

void tareaLed( void* taskParmPtr );

/* -------------------- funciones ------------------------------ */

/**
 * @fn void myTask( void* taskParmPtr )
 *
 * @brief tarea que parpadea el led con tOn incremental
 */
void tareaLed( void* taskParmPtr )
{
  TickType_t tiempoInicio;
  TickType_t tiempoBloqueo;

  TickType_t tOn, tOff;

    gpioWrite( LED2, OFF );

    gpioInit( GPIO1, GPIO_OUTPUT );

    debugPrintConfigUart( UART_USB, 115200 );
    debugPrintlnString( "Blinky con freeRTOS y sAPI." );

    tOn = TON_INICIAL;

    while(TRUE) {


        if(antirreb_tecla1.t == TECLA_PRESIONADA){
            tiempoInicio = xTaskGetTickCount();
            antirreb_tecla1.t = TECLA_SUELTA;
            gpioWrite(LEDG, ON);
            vTaskDelayUntil(&tiempoInicio, antirreb_tecla1.tiempoPresionado / portTICK_RATE_MS);
            gpioWrite(LEDG, OFF);
        }
        if(antirreb_tecla2.t == TECLA_PRESIONADA){
            tiempoInicio = xTaskGetTickCount();
            antirreb_tecla2.t = TECLA_SUELTA;
            gpioWrite(LED1, ON);
            vTaskDelayUntil(&tiempoInicio, antirreb_tecla2.tiempoPresionado / portTICK_RATE_MS);
            gpioWrite(LED1, OFF);
        }
        // vTaskDelay(10 / portTICK_RATE_MS);
/*
        tiempoInicio = xTaskGetTickCount();

        gpioWrite ( LEDR, ON );
        gpi#include "FreeRTOS.h"oWrite ( GPIO1, ON );

        tOn += TON_INCREMENTO;
        if(tOn > TON_MAX) {
            tOn = TON_MIN;
        }

        tOff = PERIODO - tOn;

        tiempoBloqueo = tOn;

        vTaskDelay(tiempoBloqueo / portTICK_RATE_MS);

        gpioWrite ( LEDR, OFF );
        gpioWrite ( GPIO1, OFF );
*/
   }
}

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{
   // ---------- CONFIGURACIONES ------------------------------
   // Inicializar y configurar la plataforma
   boardConfig();

   // UART for debug messages
   debugPrintConfigUart( UART_USB, 115200 );
   debugPrintlnString( "Blinky con freeRTOS y sAPI." );

   xTaskCreate(tareaLed, (const char *) "myTask", configMINIMAL_STACK_SIZE*2, 0,
                      tskIDLE_PRIORITY, 0);

    xTaskCreate(tareaAntirreboteTEC1, (const char*)"AR1", configMINIMAL_STACK_SIZE*2, 0,
                         tskIDLE_PRIORITY+1UL, 0);

    xTaskCreate(tareaAntirreboteTEC2, (const char*)"AR2", configMINIMAL_STACK_SIZE*2, 0,
                             tskIDLE_PRIORITY+1UL, 0);


    inicializarTecla();

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

/*==================[fin del archivo]========================================*/
