/*
 * MLT-BT05.c
 *
 *  Created on: 20 ago. 2018
 *      Author: froux
 */

/* --------------------------- includes ---------------------------------- */
#include <stdint.h>
#include <string.h>

#include "sapi.h"
#include "hm10.h"
#include "MLT-BT05.h"

CONSOLE_PRINT_ENABLE

mlt_bt05_baudrate_t MLT_BT05_inicializar (uartMap_t uart);
int32_t MLT_BT05_armarComando (char*cad, const char*cmd);

/* --------------------------- variables globales ---------------------------------- */
uint32_t mlt_bt05_baudrate [] =
    {
        115200, 57600, 38400, 19200, 9600
    };


/* --------------------------- funciones ---------------------------------- */

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
  char cad[MLT_BT05_CMD_LMAX];

  int32_t cad_l;

  for (i = BPS9600; i > 0; i--) {

      consolePrintConfigUart( uart, mlt_bt05_baudrate[i] );

      cad_l = MLT_BT05_armarComando(cad, MLT_BT05_CMD_VIVO);
      uartWriteString( uart, cad);
      respuestaModulo = waitForReceiveStringOrTimeoutBlocking( uart, MLT_BT05_CMD_VIVO_RESP, strlen(MLT_BT05_CMD_VIVO_RESP), 50 );

      if(respuestaModulo == TRUE) {
          return i;
      }


  }

  if(i < 0 )
    return BPS_ERROR;

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
  strcat(cadena, cmd);
  strcat(cadena, MLT_BT05_SUFIJO);

  strcpy( cad, cadena);

  return strlen(cmd);
}
