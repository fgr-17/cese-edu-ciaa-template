/*
 * MLT-BT05.c
 *
 *  Created on: 20 ago. 2018
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



CONSOLE_PRINT_ENABLE

mlt_bt05_baudrate_t MLT_BT05_inicializar (uartMap_t uart);
int32_t MLT_BT05_armarComando (char*cad, const char*cmd);
int32_t MLT_BT05_setBaudrate (mlt_bt05_baudrate_t baudrate);

/* --------------------------- variables globales ---------------------------------- */

MLTBT05_Baudrate_t baudrates [] = {

  {
    .brate = 1200,
    .cmdAT_baudrate = "BAUD1"
  },
  {
    .brate = 2400,
    .cmdAT_baudrate = "BAUD2"
  },
  {
    .brate = 4800,
    .cmdAT_baudrate = "BAUD3"
  },
  {
    .brate = 9600,
    .cmdAT_baudrate = "BAUD4"
  },
  {
    .brate = 19200,
    .cmdAT_baudrate = "BAUD5"
  },
  {
    .brate = 38400,
    .cmdAT_baudrate = "BAUD6"
  },
  {
    .brate = 57600,
    .cmdAT_baudrate = "BAUD7"
  },
  {
    .brate = 115200,
    .cmdAT_baudrate = "BAUD8"
  }
};


/* --------------------------- funciones ---------------------------------- */

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





/**
 * @fn void tareaEnviarArrayBLE( void* taskParmPtr )
 *
 * @brief mando mensajes d inicializacion
 *
 */

void tareaEnviarArrayBLE( void* taskParmPtr ) {

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

  xTaskCreate(tareaBLEaPC, (const char *)"BLE->PC", configMINIMAL_STACK_SIZE,(void*)0,tskIDLE_PRIORITY+1, 0);
  xTaskCreate(tareaPCaBLE, (const char *)"PC->BLE", configMINIMAL_STACK_SIZE,(void*)0,tskIDLE_PRIORITY+1, 0);

  if(baudrateInicial != BPS115200)
    MLT_BT05_setBaudrate(BPS115200);



  while(TRUE) {
      vTaskDelay(1000);
  }

}

/**
 * @fn mlt_bt05_baudrate_t MLT_BT05_inicializar (uart_t uart);
 *
 * @brief escaneo los baudrate hasta tener una respuesta del modulo
 * @intento resetearlo
 *
 */


mlt_bt05_baudrate_t MLT_BT05_inicializar (uartMap_t uart) {


  mlt_bt05_baudrate_t i;
  bool_t respuestaModulo;
  char byteRecibido;

  uartQueue_t itemQueueUartTx;
  uartQueue_t itemQueueUartRx;
  int32_t cad_l;

  cad_l = MLT_BT05_armarComando(itemQueueUartTx.mensaje, MLT_BT05_CMD_VIVO);
  i = BPS_DEFAULT;

  uartBLE.baudrate = baudrates[i].brate;
  uartInit( uartBLE.perif, uartBLE.baudrate);
  uartRxInterruptSet(UART_BLE, TRUE);

  //   uartWriteString( uart, cad);

  xQueueSend(uartBLE.queueTxUART, &itemQueueUartTx, portMAX_DELAY);
//      xQueueSend(uartPC.queueTxUART, &itemQueueUart, portMAX_DELAY);

  if(xQueueReceive(uartBLE.queueRxUART, &itemQueueUartRx, TIMEOUT_MLT_BT05_AT) == pdFALSE) {
      //      xQueueSend(uartPC.queueTxUART, &itemQueueUart, portMAX_DELAY);
  }      // pregunto si me respondió ok
  else if(strcmp(itemQueueUartRx.mensaje, MLT_BT05_CMD_VIVO_RESP) == 0) {
      return i;
  }
  else {

  }

  i = BPS_INICIO_BARRIDO;

  // for (i = BPS9600; i > 0; i--) {
  while(i != BPS_ERROR) {


      uartBLE.baudrate = baudrates[i].brate;
      uartInit( uartBLE.perif, uartBLE.baudrate);
      uartRxInterruptSet(UART_BLE, TRUE);

      //   uartWriteString( uart, cad);

      xQueueSend(uartBLE.queueTxUART, &itemQueueUartTx, portMAX_DELAY);
//      xQueueSend(uartPC.queueTxUART, &itemQueueUart, portMAX_DELAY);

      if(xQueueReceive(uartBLE.queueRxUART, &itemQueueUartRx, TIMEOUT_MLT_BT05_AT) == pdFALSE) {
          //      xQueueSend(uartPC.queueTxUART, &itemQueueUart, portMAX_DELAY);
      }      // pregunto si me respondió ok
      else if(strcmp(itemQueueUartRx.mensaje, MLT_BT05_CMD_VIVO_RESP) == 0) {
          return i;
      }
      else {

      }

      i++;
  }
  return i;

}

/**
 * @fn int32_t MLT_BT05_armarComando (char*cmd)
 *
 * @brief armo la cadena para un mensaje, retorno el largo
 *
 *
 */

int32_t MLT_BT05_armarComando (char*cad, const char*cmd) {

  char cadena[MLT_BT05_CMD_LMAX];

  strcpy(cadena, MLT_BT05_PREFIJO);
  if(strcmp(cmd, MLT_BT05_CMD_VIVO) != 0)
    strcat(cadena, MLT_BT05_CONC);
  strcat(cadena, cmd);
  strcat(cadena, MLT_BT05_SUFIJO);

  strcpy( cad, cadena);

  return strlen(cmd);
}

/**
 * @fn int32_t MLT_BT05_setBaudrate (uartMap_t uart, mlt_bt05_baudrate_t baudrate)
 *
 * @brief seteo el baudrate y ajusto la uart
 *
 *
 */

int32_t MLT_BT05_setBaudrate (mlt_bt05_baudrate_t baudrate) {

  uartQueue_t itemQueueUartTx;
  uartQueue_t itemQueueUartRx;
  int32_t cad_l;
  char cmdBaud[MENSAJE_L];
  char baudrateString[7];

  // cad_l = MLT_BT05_armarComando(itemQueueUartTx.mensaje, strcat(MLT_BT05_CMD_BAUDRATE, BAUDRATE_BLE_CMD));

//  strcpy(itemQueueUartTx.mensaje, baudrates[baudrate].cmdAT_baudrate);}

  MLT_BT05_armarComando(itemQueueUartTx.mensaje, baudrates[baudrate].cmdAT_baudrate );
  xQueueSend(uartBLE.queueTxUART, &itemQueueUartTx, portMAX_DELAY);
 //      xQueueSend(uartPC.queueTxUART, &itemQueueUart, portMAX_DELAY);

  if(xQueueReceive(uartBLE.queueRxUART, &itemQueueUartRx, TIMEOUT_MLT_BT05_AT) == pdFALSE) {
      strcpy(itemQueueUartTx.mensaje, "No se recibio respuesta de parte del BLE\r\n");
      xQueueSend(uartPC.queueTxUART, &itemQueueUartTx, portMAX_DELAY);
      return -1;
  }      // pregunto si me respondió ok

  // armo string con el valor del baudrate configurado
  itoa(baudrates[baudrate].brate, baudrateString, 10);
  // armo el string de lo que deberia responder el BLE
  strcpy(cmdBaud, MLT_BT05_CAMBIO_BAUDRATE_RESP1);
  strcat(cmdBaud, baudrateString);
  strcat(cmdBaud, MLT_BT05_CAMBIO_BAUDRATE_RESP2);

  if(strcmp(cmdBaud, itemQueueUartRx.mensaje) == 0) {
    // se configuro todo correctamente
    uartBLE.baudrate = baudrates[baudrate].brate;
    uartInit( uartBLE.perif, uartBLE.baudrate);
    uartRxInterruptSet(UART_BLE, TRUE);

    strcpy(itemQueueUartTx.mensaje, "Modulo BLE configurado a ");
    strcat(itemQueueUartTx.mensaje, baudrateString);
    strcat(itemQueueUartTx.mensaje, "bps\r\n");
    xQueueSend(uartPC.queueTxUART, &itemQueueUartTx, portMAX_DELAY);
    return 0;
  }
  else {
      strcpy(itemQueueUartTx.mensaje, "Error al configurar baudrate del BLE\r\n");
      xQueueSend(uartPC.queueTxUART, &itemQueueUartTx, portMAX_DELAY);
      return -2;
  }

}
