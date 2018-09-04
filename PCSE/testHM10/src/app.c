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
#include "app.h"

/* --------------------------- variables ---------------------------------- */

estadoApp_t estadoApp;
estadoAplicacionBLE_t estadoAplicacionBLE;

QueueHandle_t queueADC;

TaskHandle_t pTareaPCaBLE;
TaskHandle_t pTareaBLEaPC;
TaskHandle_t pTareaSimularMuestreo;
TaskHandle_t pTareaAplicacionBLE;

/** ==================[ funciones ]====================*/


void salirModoPasamanos ( void );
void ingresarModoPasamanos ( void );
void salirModoComando ( void );
void ingresarModoComando ( void );
int8_t  validarPaquete (uartQueue_t itemQueue, paq_t* paq);
int8_t  armarPaqueteRespuestaComando (uartQueue_t*itemQueue, cmd_t cmd, resp_t respuesta);

/** ==================[ tarea ]====================*/

void tareaSimularMuestreo ( void*taskParam );
int32_t inicializarAplicacionBLE( void );
void tareaAplicacionBLE ( void* taskParmPtr );
void tareaInicializarSistema( void* taskParmPtr );
void tareaBLEaPC( void* taskParmPtr );
/** ==================[ señales ]====================*/

uint16_t arrayDatos [] = {
  #include "sgn1.h"
};

uint16_t abcdario[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

#define ARRAY_DATOS_L   (sizeof(arrayDatos) / sizeof(uint16_t))
#define ABCDARIO_L    (sizeof(abcdario) / sizeof(uint16_t))

/* --------------------------- funciones : ---------------------------------- */

/**
 * @fn void salirModoPasamanos (void)
 *
 * @brief configuro la uart para cualquier cant. de bytes
 * y habilito tareas caño
 *
 */

void salirModoPasamanos ( void ) {

  uartQueue_t itemQueue;
  taskENTER_CRITICAL();

  configurarUARTModoBytes(&uartPC);
  configurarUARTModoBytes(&uartBLE);

  strcpy(itemQueue.mensaje, "Salgo de modo pasamanos\r\n");
  xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );

  taskENTER_CRITICAL();
  vTaskSuspend(pTareaBLEaPC);
  vTaskSuspend(pTareaPCaBLE);
  taskEXIT_CRITICAL();

  return;
}

/**
 * @fn void ingresarModoPasamanos (void)
 *
 * @brief configuro la uart para cualquier cant. de bytes
 * y habilito tareas caño
 *
 */

void ingresarModoPasamanos ( void ) {
  static int32_t primerIngreso = 1;
  uartQueue_t itemQueue;

  //xTaskSuspend();


  configurarUARTModoBytes(&uartPC);
  configurarUARTModoBytes(&uartBLE);

  strcpy(itemQueue.mensaje, "Ingreso a modo pasamanos\r\n");
  xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );

  taskENTER_CRITICAL();
  if(primerIngreso) {
    primerIngreso = 0;
    xTaskCreate(tareaBLEaPC, (const char *)"BLE->PC", configMINIMAL_STACK_SIZE,(void*)0,tskIDLE_PRIORITY+1, (TaskHandle_t*const) &pTareaBLEaPC);
    xTaskCreate(tareaPCaBLE, (const char *)"PC->BLE", configMINIMAL_STACK_SIZE,(void*)0,tskIDLE_PRIORITY+1, (TaskHandle_t*const) &pTareaPCaBLE);
  }
  else {
    vTaskResume(tareaBLEaPC);
    vTaskResume(tareaPCaBLE);
  }

  taskEXIT_CRITICAL();

  return;
}


/**
 * @fn void salirModoPasamanos (void)
 *
 * @brief configuro la uart para cualquier cant. de bytes
 * y habilito tareas caño
 *
 */

void salirModoComando ( void ) {

  uartQueue_t itemQueue;
  configurarUARTModoBytes(&uartPC);
  configurarUARTModoBytes(&uartBLE);

  strcpy(itemQueue.mensaje, "Salgo de modo comando\r\n");
  xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );

  taskENTER_CRITICAL();
  vTaskSuspend(pTareaSimularMuestreo);
  vTaskSuspend(pTareaAplicacionBLE);
  taskEXIT_CRITICAL();
  return;
}
/**
 * @fn void ingresarModoComando (void)
 *
 * @brief configuro la uart para cant. de bytes fija
 * y habilito tareas de aplicacion
 *
 */

void ingresarModoComando ( void ) {
  static int32_t primerIngreso = 1;

  uartQueue_t itemQueue;

  taskENTER_CRITICAL();

  strcpy(itemQueue.mensaje, "Ingreso a modo Comando\r\n");
  xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );

  vTaskDelay(500);

  configurarUARTModoUmbral(&uartPC);
  configurarUARTModoUmbral(&uartBLE);

  if(primerIngreso) {
    xTaskCreate(tareaSimularMuestreo, (const char *)"Muestreo", configMINIMAL_STACK_SIZE,(void*)0,tskIDLE_PRIORITY+1, (TaskHandle_t*const) &pTareaSimularMuestreo);
    xTaskCreate(tareaAplicacionBLE, (const char *)"appBLE", configMINIMAL_STACK_SIZE,(void*)0,tskIDLE_PRIORITY+1, (TaskHandle_t*const) &pTareaAplicacionBLE);
    primerIngreso = 0;
  }
  else {
      vTaskResume(tareaSimularMuestreo);
      vTaskResume(tareaAplicacionBLE);
  }


  taskEXIT_CRITICAL();

  return;
}

/**
 * @fn paq_t validarPaquete (uartQueue_t itemQueue)
 *
 * @brief copio el dato traido por la uart a un paquete y valido el comando
 *
 */

int8_t  validarPaquete (uartQueue_t itemQueue, paq_t* paq) {

  int8_t chksum;

  int8_t i;

  chksum = 0;

  for ( i = 0; i < MENSAJE_L; i++ ) {

      paq->bytes[i] = itemQueue.mensaje[i];
      chksum ^= paq->bytes[i];
  }

  return chksum;

}


/**
 * @fn int8_t  armarPaquete (uartQueue_t*itemQueue, cmd_t cmd)
 *
 *
 *
 * @brief armo cadena a enviar a partir de un comando y datos
 *
 */

int8_t  armarPaqueteRespuestaComando (uartQueue_t*itemQueue, cmd_t cmd, resp_t respuesta) {

  int8_t chksum;

  switch(respuesta) {
    case RESP_OK:
    chksum = cmd ^ 'O' ^ 'K';
    itemQueue->mensaje[0] = cmd;
    itemQueue->mensaje[1] = 'O';
    itemQueue->mensaje[2] = 'K';
    itemQueue->mensaje[3] = chksum;
    break;

    case RESP_ERR_CMD:
      chksum = cmd ^ 'N' ^ 'C';
      itemQueue->mensaje[0] = cmd;
      itemQueue->mensaje[1] = 'N';
      itemQueue->mensaje[2] = 'C';
      itemQueue->mensaje[3] = chksum;
      break;

    default:
      return 1;
  }
  return 0;
}




/* --------------------------- tareas e instaladores : ---------------------------------- */


/**
 * @fn void tareaSimularMuestreo ( void )
 *
 * @brief tarea que manda buffer por una queue.
 * cambiar tamaño para diferentes Fs.
 *
 */

void tareaSimularMuestreo ( void*taskParam ) {

  itemQueueADC_t itemQueueADC;
  int32_t i_adc, i_sgn;

  int16_t*pAux;

  /* inicializo punteros a buffer */
  pAct = buf1;
  pEnv = buf2;



  while (TRUE) {


      if(estadoAplicacionBLE == ENVIO_DATOS) {

        i_adc = 0;
        i_sgn = 0;

        // cargo valores hasta completar buffer del ADC
        while(i_adc < BUF_ADC_L) {

            *pAct = arrayDatos[i_sgn];
            pAct++;
            i_sgn++;
            i_adc++;

            if(i_sgn > ARRAY_DATOS_L) {
                i_sgn = 0;
            }
        }

        itemQueueADC.pEnvio = pAct;
        pAux = pAct;
        pAct = pEnv;
        pEnv = pAct;

        xQueueSend(queueADC, &itemQueueADC, portMAX_DELAY);
        vTaskDelay(TAREA_SIMULAR_MUESTREO_DELAY);
      }
      vTaskDelay(2000);
  }

}

/**
 * @fn int32_t inicializarAplicacionBLE( void )
 *
 * @brief instalo la aplicacion BLE
 *
 */

int32_t inicializarAplicacionBLE( void ) {

  estadoAplicacionBLE = ESPERANDO_LOGIN;

  /* inicializo los buffers del ADC */
  pAct = buf1;
  pEnv = buf2;

  queueADC = xQueueCreate(QUEUE_ADC_L, sizeof(itemQueueADC_t));

  return 0;
}

/**
 * @fn void tareaAplicacion( void* taskParmPtr )
 *
 * @brief aplicacion
 *
 */



void tareaAplicacionBLE ( void* taskParmPtr ) {

  uartQueue_t itemQueue;
  paq_t paqueteValidado;
  uartWord_Byte_t usuario;
  itemQueueADC_t itemQueueADC;

  inicializarAplicacionBLE();

  while ( TRUE ) {

      if(estadoApp == MODO_CMD)
      {
          xQueueReceive(uartBLE.queueRxUART, &itemQueue, portMAX_DELAY );

          if(!validarPaquete(itemQueue, &paqueteValidado)) {

            switch (estadoAplicacionBLE) {

              case ESPERANDO_LOGIN:

                if(paqueteValidado.frame.cmd == LOGIN) {
                    armarPaqueteRespuestaComando(&itemQueue, paqueteValidado.frame.cmd, RESP_OK);
                    xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY);
                    estadoApp = ESPERANDO_USUARIO;
                }
                else {
                    armarPaqueteRespuestaComando(&itemQueue, paqueteValidado.frame.cmd, RESP_ERR_CMD);
                    xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY);
                    estadoApp = ESPERANDO_LOGIN;
                }
                break;

              case ESPERANDO_USUARIO:

                if(paqueteValidado.frame.cmd == USUARIO) {
                    armarPaqueteRespuestaComando(&itemQueue, paqueteValidado.frame.cmd, RESP_OK);
                    xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY);
                    usuario.bytes[1] = paqueteValidado.bytes[1];
                    usuario.bytes[0] = paqueteValidado.bytes[0];
                    estadoApp = ESPERO_HYF;
                }
                else {
                    armarPaqueteRespuestaComando(&itemQueue, paqueteValidado.frame.cmd, RESP_ERR_CMD);
                    xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY);
                    estadoApp = ESPERANDO_USUARIO;
                }
                break;

              case ESPERO_HYF:

                if(paqueteValidado.frame.cmd == HYF) {
                    armarPaqueteRespuestaComando(&itemQueue, paqueteValidado.frame.cmd, RESP_OK);
                    xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY);
                    estadoApp = ESPERO_INICIO_LOG;
                }
                else {
                    armarPaqueteRespuestaComando(&itemQueue, paqueteValidado.frame.cmd, RESP_ERR_CMD);
                    xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY);
                    estadoApp = ESPERO_HYF;
                }
                break;

              case ESPERO_INICIO_LOG:
                if(paqueteValidado.frame.cmd == INICIAR_LOG) {
                    armarPaqueteRespuestaComando(&itemQueue, paqueteValidado.frame.cmd, RESP_OK);
                    xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY);
                    estadoApp = ESPERO_INICIO_LOG;
                }
                else {
                    armarPaqueteRespuestaComando(&itemQueue, paqueteValidado.frame.cmd, RESP_ERR_CMD);
                    xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY);
                    estadoApp = ESPERO_HYF;
                }
                break;

              case ENVIO_DATOS:

                if(paqueteValidado.frame.cmd == FIN_LOG) {
                    armarPaqueteRespuestaComando(&itemQueue, paqueteValidado.frame.cmd, RESP_OK);
                    xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY);
                    estadoApp = ESPERO_INICIO_LOG;
                }
                else {
                  xQueueReceive(queueADC, &itemQueueADC, portMAX_DELAY);
                }
                break;






              default:
                inicializarAplicacionBLE();
            }
          }
          else {

              // error de checksum
          }

      }
      else
        vTaskDelay ( 500 );

  }

}


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

  vTaskDelay(200);

  if(baudrateInicial != BPS115200)
    MLT_BT05_setBaudrate(BPS115200);

  strcpy(itemQueue.mensaje, "Sistema inicializado\r\nPresione TEC1 para cambiar al modo pasamanos\r\n");
  xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );

  vTaskDelay(200);

  ingresarModoComando();
  estadoApp = MODO_CMD;

  while(TRUE) {


      switch(estadoApp)
      {

        case MODO_CMD:

          if(gpioRead(TEC1) == OFF) {
              salirModoComando();
              ingresarModoPasamanos();
              estadoApp = MODO_PASAMANOS;
              vTaskDelay (300);
          }
          break;


        case MODO_PASAMANOS:

          if(gpioRead(TEC1) == OFF) {
              salirModoPasamanos();
              ingresarModoComando();
              estadoApp = MODO_CMD;
              vTaskDelay (300);
          }

          break;

        default:
          estadoApp = MODO_CMD;

      }

      vTaskDelay(300);
  }

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

    if(estadoApp == MODO_PASAMANOS) {
      xQueueReceive(uartBLE.queueRxUART, &itemQueue, portMAX_DELAY );
      xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );
    }
    else {
        vTaskDelay(500);
    }

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

    if(estadoApp == MODO_PASAMANOS) {
      xQueueReceive(uartPC.queueRxUART, &itemQueue, portMAX_DELAY );
      xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY );
    }
      else {
          vTaskDelay(500);
    }
  }
}
