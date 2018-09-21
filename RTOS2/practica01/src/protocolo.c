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

/* ---------------------------- includes --------------------------------- */

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sapi.h"

#include "qmpool.h"

#include "uart.h"
#include "protocolo.h"



/* ---------------------------- protocolos de funciones --------------------------------- */

int32_t inicializarRecibirPaquete (void);
void tareaMayusculizar (void*taskPtr);
void tareaEnviarMayusculizados (void*taskPtr);
void tareaRecibirPaquete (void* taskParam);

static int32_t validarOP (op_t op);
static uint8_t* clasificarTamPool (uint8_t tam, QMPool*qmpool);
static int32_t procesarDatos(uint8_t*pPool, QMPool*qmpool, uint8_t tam, op_t op);

/* ---------------------------- colas de dato --------------------------------- */

QueueHandle_t queMayusculizar;
QueueHandle_t queMinusculizar;
QueueHandle_t queMayusculizados;
QueueHandle_t queMinusculizados;

/* ---------------------------- pools de memoria --------------------------------- */

QMPool memPoolS;
static uint8_t poolMemoriaS[POOL_MEMORIA_S_T]; /* Espacio de almacenamiento para el Pool */

QMPool memPoolM;
static uint8_t poolMemoriaM[POOL_MEMORIA_M_T]; /* Espacio de almacenamiento para el Pool */

QMPool memPoolL;
static uint8_t poolMemoriaL[POOL_MEMORIA_L_T]; /* Espacio de almacenamiento para el Pool */

/* ---------------------------- variables globales --------------------------------- */

estadoRecepcion_t estadoRecepcion;

/* ---------------------------- implementacion de funciones --------------------------------- */


/**
 * @fn int32_t inicializarQueues (void)
 *
 * @brief inicializo las colas de datos
 *
 */
int32_t inicializarQueues (void) {

  queMayusculizar = xQueueCreate (QUEUE_MAYUSCULIZAR_L, sizeof(queueMayMin_t));
  queMinusculizar = xQueueCreate (QUEUE_MINUSCULIZAR_L, sizeof(queueMayMin_t));
  queMayusculizados = xQueueCreate (QUEUE_MAYUSCULIZADOS_L, sizeof(queueMayMin_t));
  queMinusculizados = xQueueCreate (QUEUE_MINUSCULIZADOS_L, sizeof(queueMayMin_t));

  return 0;
}

/**
 * @fn int32_t inicializarRecibirPaquete (void)
 *
 * @brief inicializo la tarea
 *
 */
int32_t inicializarRecibirPaquete (void) {
  estadoRecepcion = RECIBIR_STX;

  // inicializo las colas de dato:
  inicializarQueues();

  // inicializo los 3 pools de memoria :
  QMPool_init(&memPoolS, poolMemoriaS, sizeof(poolMemoriaS), POOL_MEMORIA_S_TBLOQUE);
  QMPool_init(&memPoolM, poolMemoriaM, sizeof(poolMemoriaM), POOL_MEMORIA_M_TBLOQUE);
  QMPool_init(&memPoolL, poolMemoriaL, sizeof(poolMemoriaL), POOL_MEMORIA_L_TBLOQUE);


  return 0;
}

/**
 * @fn void tareaMayusculizar (void*taskPtr)
 *
 * @brief tarea que espera bloqueada una queue para pasar caracteres en minus a mayus y
 *        ponerlo en una cola de salida
 */

void tareaMayusculizar (void*taskPtr) {

  queueMayMin_t itemQueue;

  uint8_t i;

  while( TRUE ) {

      xQueueReceive(queMayusculizar, &itemQueue, portMAX_DELAY);

      for(i = PRT_DAT_INI_I; i < (itemQueue.bufL + PRT_DAT_INI_I); i++ )
          if( (itemQueue.buf[i] >= 'a') && (itemQueue.buf[i] <= 'z') )
            itemQueue.buf[i] -= 'a' - 'A';

      xQueueSend(queMayusculizados, &itemQueue, portMAX_DELAY);

  }

}

/**
 * @fn void tareaEnviarMayusculizados (void*taskPtr)
 *
 * @brief tarea que espera bloqueada una queue con datos en mayuscula
 */

void tareaEnviarMayusculizados (void*taskPtr) {

  queueMayMin_t itemQueue;

  uint8_t i;

  while( TRUE ) {

      // espero recibir cadena con mayusculas mientras estoy bloqueado
      xQueueReceive(queMayusculizados, &itemQueue, portMAX_DELAY);

      // funcion que vacia buffer en fifo de tx de la uart a medida que se va vaciando.
      // vuelve cuando termino
      descargarBufferEnFIFOUARTTx(uartPC.perif, itemQueue.buf, itemQueue.bufL + PRT_BYTES_PROTCOLO);

      // una vez que procese los datos, libero el pool recibido
      //QMPool_put(itemQueue.ctrlPool, itemQueue.buf);
  }
}

/**
 * @fn void tareaRecibirPaquete (void* taskParam)
 *
 * @brief tarea con máquina de estados para procesar el paquete
 *
 */
void tareaRecibirPaquete (void* taskParam) {

  // variable temporal del byte recibido
  uint8_t byteRecibido;
  // contador de bytes recibidos y tamaño total
  uint8_t cuentaT = 0;
  uint8_t tam;

  // punteros a pool y estructura de control
  uint8_t*pPool;
  uint8_t*pDatos;
  QMPool*qmpool;

  // operacion recibida
  op_t operacion;


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

        if(validarOP(byteRecibido) != -1) {
            // guardo el byte de operación
            operacion = (op_t)byteRecibido;
            estadoRecepcion = RECIBIR_T;
        }
        else {
            estadoRecepcion = RECIBIR_STX;
        }
        break;

      case RECIBIR_T:
        // no valido el dato porque puedo recibir cualquier numero entre 0 y 255
        cuentaT = byteRecibido;
        tam = byteRecibido;
        // reservo un pool de memoria del tamaño adecuado
        pPool = clasificarTamPool(cuentaT, qmpool);
        // me hago una copia del puntero para guardar el inicio
        pDatos = pPool;

        *pDatos = PRT_STX;
        pDatos++;
        *pDatos = operacion;
        pDatos++;
        *pDatos = tam;
        pDatos++;

        estadoRecepcion = RECIBIENDO_DATOS;
        break;

      case RECIBIENDO_DATOS:

        *pDatos = byteRecibido;
        pDatos++;
        cuentaT--;

        if(cuentaT == 0)
          estadoRecepcion = RECIBIR_ETX;
        else
          estadoRecepcion = RECIBIENDO_DATOS;

        break;

      case RECIBIR_ETX:


        if(byteRecibido == PRT_ETX) {
          *pDatos = PRT_ETX;
          procesarDatos(pPool, qmpool, tam, operacion);
        }
        else
          // si no recibí ETX, descarto los datos recibidos
          QMPool_put(qmpool, pPool);

        estadoRecepcion = RECIBIR_STX;
        break;



      default:
        inicializarRecibirPaquete();

    }
  }
}


/**
 * @fn static int32_t validarOP (op_t op)
 *
 * @brief valido los codigos de operacion
 *
 */
static int32_t validarOP (op_t op) {

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

/**
 * @fn uint8_t* clasificarTamPool (uint8_t tam)
 *
 * @brief recibo el tamaño del buffer a procesar y devuelvo un pool de tamaño adecuado
 */

static uint8_t* clasificarTamPool (uint8_t tam, QMPool*qmpool) {

  uint8_t*pPool;

  // al tam recibido, le sumo 4 bytes por el STX, OP, TAM y ETX
  tam = tam + PRT_BYTES_PROTCOLO;

  if(tam <= POOL_MEMORIA_S_TBLOQUE) {
    pPool = QMPool_get(&memPoolS, 0U);
    qmpool = &memPoolS;
  }
  else if((tam > POOL_MEMORIA_S_TBLOQUE) && (tam <= POOL_MEMORIA_M_TBLOQUE)) {
    pPool = QMPool_get(&memPoolM, 0U);
    qmpool = &memPoolM;
  }
  else if(tam > POOL_MEMORIA_M_TBLOQUE) {
    pPool = QMPool_get(&memPoolL, 0U);
    qmpool = &memPoolL;
  }
  return pPool;
}

/**
 * @fn int32_t procesarDatos(uint8_t*pPool, uint8_t tam, op_t op);
 *
 * @brief recibo el pool y su longitud, la operación, y proceso el texto
 */

static int32_t procesarDatos(uint8_t*pPool, QMPool*qmpool, uint8_t tam, op_t op) {

  queueMayMin_t itemQueue;


  switch(op){
    case PRT_MAYUS:
      itemQueue.buf = pPool;
      itemQueue.ctrlPool = qmpool;
      itemQueue.bufL = tam;
      xQueueSend(queMayusculizar, &itemQueue, portMAX_DELAY);
      break;
    case PRT_MINUS:
      itemQueue.buf = pPool;
      itemQueue.ctrlPool = qmpool;
      itemQueue.bufL = tam;
      xQueueSend(queMinusculizar, &itemQueue, portMAX_DELAY);
      break;
    case PRT_REPSTACK:
      break;
    case PRT_REPHEAP:
      break;
    case PRT_MSJEST:
      break;
    default:
      return -1;
  }

}


