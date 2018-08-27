/*
 * app.c
 *
 *  Created on: 24 ago. 2018
 *      Author: froux
 */

/* --------------------------- includes ---------------------------------- */

#include <string.h>
#include <stdint.h>

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sapi.h"        // <= Biblioteca sAPI

#include "hm10.h"

#include "uart.h"

#include "MLT-BT05.h"

/* --------------------------- tareas ---------------------------------- */

void tareaInicializarSistema( void* taskParmPtr );
void tareaBLEaPC( void* taskParmPtr );
void tareaPCaBLE( void* taskParmPtr );

/* --------------------------- prototipos ---------------------------------- */

void ingresarModoPasamanos ( void );

/* --------------------------- variables ---------------------------------- */

typedef enum {MODO_CMD, MODO_PASAMANOS} estadoApp_t;

estadoApp_t estadoApp;

/* --------------------------- tareas ---------------------------------- */

/**
 * @fn void tareaInicializarSistema( void* taskParmPtr )
 *
 * @brief mando mensajes d inicializacion
 *
 */

void tareaInicializarSistema( void* taskParmPtr ) {

  uartQueue_t itemQueue;
  mlt_bt05_baudrate_t baudrateInicial;

  char baudrateString[7];

  baudrateInicial = MLT_BT05_inicializar(uartBLE.perif);

  if( baudrateInicial == BPS_ERROR) {
      strcpy(itemQueue.mensaje, "Error al inicializar el modulo BLE\r\n");
      xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );
      configASSERT(0);
  }

  itoa(baudrates[baudrateInicial].brate, baudrateString, 10);

  strcpy(itemQueue.mensaje, "Modulo BLE inicializado a ");
  strcat(itemQueue.mensaje, baudrateString);
  strcat(itemQueue.mensaje, "bps\r\n");
  xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );


  if(baudrateInicial != BPS115200)
    MLT_BT05_setBaudrate(BPS115200);


  strcpy(itemQueue.mensaje, "Sistema inicializado\r\nPresione TEC1 para cambiar al modo pasamanos\r\n");
  xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );





  estadoApp = MODO_CMD;

  while(TRUE) {


      switch(estadoApp)
      {

        case MODO_CMD:

          if(gpioRead(TEC1) == ON) {
              ingresarModoPasamanos();
              vTaskDelay (300);
          }
          break;


        case MODO_PASAMANOS:

          if(gpioRead(TEC1) == ON) {


          }


          break;

        default:
          estadoApp = MODO_CMD;

      }

      vTaskDelay(1000);
  }

}

/**
 * @fn void ingresarModoPasamanos (void)
 *
 * @brief configuro la uart para cualquier cant. de bytes
 * y habilito tareas caño
 *
 */

void ingresarModoPasamanos ( void )
{
  uartQueue_t itemQueue;

  xTaskCreate(tareaBLEaPC, (const char *)"BLE->PC", configMINIMAL_STACK_SIZE,(void*)0,tskIDLE_PRIORITY+1, 0);
  xTaskCreate(tareaPCaBLE, (const char *)"PC->BLE", configMINIMAL_STACK_SIZE,(void*)0,tskIDLE_PRIORITY+1, 0);

  configurarUARTModoBytes(&uartPC);
  configurarUARTModoBytes(&uartBLE);

  strcpy(itemQueue.mensaje, "Ingreso a modo pasamanos\r\nPresione TEC1 para volver al modo comando\r\n");
  xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );


  return;
}


/**
 * @fn void tareaPCaBLE( void* taskParmPtr )
 *
 * @brief caño de la pc al ble
 *
 */

void tareaBLEaPC( void* taskParmPtr ) {

  uartQueue_t itemQueue;
  while(TRUE) {

      xQueueReceive(uartBLE.queueRxUART, &itemQueue, portMAX_DELAY );
      xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );

  }
}

/**
 * @fn void tareaBLEaPC( void* taskParmPtr )
 *
 * @brief caño del ble a la pc
 *
 */

void tareaPCaBLE( void* taskParmPtr ) {

  uartQueue_t itemQueue;
  while(TRUE) {

      xQueueReceive(uartPC.queueRxUART, &itemQueue, portMAX_DELAY );
      xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY );

  }
}
