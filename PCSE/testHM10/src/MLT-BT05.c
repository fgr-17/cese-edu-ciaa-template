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
        .brate = 9600,
        .cmdAT_baudrate = "BAUD4"
    },
    {
        .brate = 19200,
        .cmdAT_baudrate = "BAUD3"
    },
    {
        .brate = 38400,
        .cmdAT_baudrate = "BAUD2"
    },
    {
        .brate = 57600,
        .cmdAT_baudrate = "BAUD1"
    },
    {
        .brate = 115200,
        .cmdAT_baudrate = "BAUD0"
    }
};


/* --------------------------- funciones ---------------------------------- */


/**
 * @fn mlt_bt05_baudrate_t MLT_BT05_inicializar (uart_t uart);
 *
 * @brief escaneo los baudrate hasta tener una respuesta del modulo
 * @intento resetearlo
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

  strcpy(itemQueue.mensaje, "Modulo BLE inicializado en ");
  strcat(itemQueue.mensaje, baudrateString);
  strcat(itemQueue.mensaje, "bps\r\n");
  xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );

  //MLT_BT05_setBaudrate(BPS115200);


  while(TRUE) {

      xQueueReceive(uartPC.queueRxUART, &itemQueue, portMAX_DELAY );
      xQueueSend(uartBLE.queueTxUART, &itemQueue, portMAX_DELAY );

      xQueueReceive(uartBLE.queueRxUART, &itemQueue, portMAX_DELAY );
      xQueueSend(uartPC.queueTxUART, &itemQueue, portMAX_DELAY );

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
  i = BPS9600;
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

      i--;
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
  char cmd_baud[MENSAJE_L];

  // cad_l = MLT_BT05_armarComando(itemQueueUartTx.mensaje, strcat(MLT_BT05_CMD_BAUDRATE, BAUDRATE_BLE_CMD));

//  strcpy(itemQueueUartTx.mensaje, baudrates[baudrate].cmdAT_baudrate);}

  MLT_BT05_armarComando(cmd_baud, baudrates[baudrate].cmdAT_baudrate );
  xQueueSend(uartBLE.queueTxUART, &itemQueueUartTx, portMAX_DELAY);
 //      xQueueSend(uartPC.queueTxUART, &itemQueueUart, portMAX_DELAY);

  if(xQueueReceive(uartBLE.queueRxUART, &itemQueueUartRx, TIMEOUT_MLT_BT05_AT) == pdFALSE) {
      strcpy(itemQueueUartTx.mensaje, "No se recibio respuesta de parte del BLE");
      xQueueSend(uartPC.queueTxUART, &itemQueueUartTx, portMAX_DELAY);
      return -1;
  }      // pregunto si me respondió ok

  uartBLE.baudrate = baudrates[baudrate].brate;
  uartInit( uartBLE.perif, uartBLE.baudrate);
  uartRxInterruptSet(UART_BLE, TRUE);

  /*
  else if(strcmp(itemQueueUartRx.mensaje, MLT_BT05_CMD_VIVO_RESP) == 0) {
     return i;
  }
  else {

  }
*/


}
