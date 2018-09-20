/*
 * protocolo.c
 *
 *  Created on: 18 sep. 2018
 *      Author: froux
 *
 *      usuarios gitlab profesores:
 *
 *
 *       @sergiordj
 *       @fbucafusco
 *
 */

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sapi.h"
#include "uart.h"
#include "protocolo.h"

#include "qmpool.h"
estadoRecepcion_t estadoRecepcion;

int32_t inicializarRecibirPaquete (void);
void tareaRecibirPaquete (void* taskParam);


/**
 * @fn int32_t inicializarRecibirPaquete (void)
 *
 * @brief inicializo la tarea
 *
 */
int32_t inicializarRecibirPaquete (void) {
  estadoRecepcion = RECIBIR_STX;
  return 0;
}

/**
 * @fn void tareaRecibirPaquete (void* taskParam)
 *
 * @brief tarea con máquina de estados para procesar el paquete
 *
 */
void tareaRecibirPaquete (void* taskParam) {

  uint8_t byteRecibido;
  uint8_t cuentaT = 0;

  while(TRUE) {


    xQueueReceive(uartPC.queueRxUART, &byteRecibido, TIMEOUT_RX_PAQUETE);

    switch(estadoRecepcion) {

      case RECIBIR_STX:

        if(byteRecibido == PRT_STX) {
            estadoRecepcion = RECIBIR_OP;
        }
        else {
            estadoRecepcion = RECIBIR_STX;
        }
        break;


      case RECIBIR_OP:
        if(validarOp(byteRecibido) != -1) {
            estadoRecepcion = RECIBIR_T;
        }
        else {
            estadoRecepcion = RECIBIR_STX;
        }
        break;

      case RECIBIR_T:
        cuentaT = byteRecibido;

        estadoRecepcion = RECIBIENDO_DATOS;


        break;

      case RECIBIENDO_DATOS:


        break;

      case RECIBIR_ETX:
        break;

      default:
        inicializarRecibirPaquete();


    }
  }


}

int32_t validarOP (op_t op) {

  switch(op){
    case PRT_MAYUS:
    case PRT_MINUS:
    case PRT_REPSTACK:
    case PRT_REPHEAP:
    case PRT_MSJEST:
      return op;
    default:
      return -1;
  }

}
