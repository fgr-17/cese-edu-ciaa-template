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
#include "FreeRTOSConfig.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// sAPI header
#include "sapi.h"

#include "qmpool.h"

#include "gpio.h"
#include "uart.h"



/* ---- include para manejo de framework basado por eventos ---- */
#include "FrameworkEventos.h"
#include "manejoEventos.h"
#include "protocolo.h"
#include "antirreb.h"

/*==================[definiciones de datos externos]=========================*/


/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{

  boardConfig();
  inicializarTareaEnviarDatosUARTs();
  inicializarRecibirPaquete();
  inicializarTimer();

  inicializarColasEventos();
  inicializarModulos();

  /* ----------- instalacion de tareas -------------- */

  // manejo del protocolo
  xTaskCreate(tareaRecibirPaquete,(const char *)"recibopaq", configMINIMAL_STACK_SIZE, (void*) 0, tskIDLE_PRIORITY+3, 0);

  // procesamiento de datos
  xTaskCreate(tareaMayusculizar,(const char *)"mayusculizar", configMINIMAL_STACK_SIZE, (void*) 0, tskIDLE_PRIORITY+2, 0);
  xTaskCreate(tareaMinusculizar,(const char *)"minusc", configMINIMAL_STACK_SIZE, (void*) 0, tskIDLE_PRIORITY+2, 0);
  xTaskCreate(tareaMedirPerformance,(const char *)"mperf", configMINIMAL_STACK_SIZE, (void*) 0, tskIDLE_PRIORITY+2, 0);

  // salida de la uart
  xTaskCreate(tareaTransmisionUART,(const char *)"salidaUART", configMINIMAL_STACK_SIZE, (void*) 0, tskIDLE_PRIORITY+1, 0);

  // SISTEMA REACTIVO:
  xTaskCreate(taskDespacharEventos, (const char*) "dispatcher", configMINIMAL_STACK_SIZE*5, (void*) queEventosBaja, tskIDLE_PRIORITY + 1, NULL);

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
