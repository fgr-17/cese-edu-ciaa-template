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
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sapi.h"

#include "qmpool.h"

#include "uart.h"

#include "protocolo.h"
/* ---------------------------- protocolos de funciones --------------------------------- */


char* itoa(int value, char* result, int base);

int32_t inicializarRecibirPaquete (void);

void tareaMayusculizar (void*taskPtr);
void tareaEnviarMayusculizados (void*taskPtr);
void tareaRecibirPaquete (void* taskParam);
void tareaMinusculizar (void*taskPtr);
void tareaEnviarMinusculizados (void*taskPtr);

uint32_t liberarPoolMasAntiguo (void);
uint8_t armarPaqueteMedicionStack (uint8_t*buf, UBaseType_t stackMedido);

static uint8_t armarPaqueteMedicionHeap (uint8_t*buf);

static int32_t validarOP (op_t op);
static int32_t procesarDatos(poolInfo_t*poolAsociado, op_t op);
static poolInfo_t* obtenerPoolPorTam (uint8_t tam);

static int32_t inicializarColaCeldaPool(colaCeldaPool_t*colaPool);
static poolInfo_t* obtenerCeldaPoolParaEscribir(colaCeldaPool_t*colaCeldaPool);
static poolInfo_t* obtenerCeldaPoolParaLiberar(colaCeldaPool_t*colaCeldaPool);
static poolInfo_t* obtenerCeldaPoolParaDescartar(colaCeldaPool_t*colaCeldaPool);

static uint32_t liberarPoolMasReciente (void);
static poolInfo_t* obtenerPoolPorTam (uint8_t tam);

/* ---------------------------- colas de dato --------------------------------- */

QueueHandle_t queMayusculizar;
QueueHandle_t queMinusculizar;

QueueHandle_t queMedirPerformance;

//QueueHandle_t queTransmision;

/* ---------------------------- pools de memoria --------------------------------- */

QMPool memPoolS;
static uint8_t poolMemoriaS[POOL_MEMORIA_S_T]; /* Espacio de almacenamiento para el Pool */

QMPool memPoolM;
static uint8_t poolMemoriaM[POOL_MEMORIA_M_T]; /* Espacio de almacenamiento para el Pool */

QMPool memPoolL;
static uint8_t poolMemoriaL[POOL_MEMORIA_L_T]; /* Espacio de almacenamiento para el Pool */

QMPool memPoolToken;
static uint8_t poolMemoriaToken[POOL_MEMORIA_TOKEN_T]; /* Espacio de almacenamiento para el Pool de tokens */

/* ---------------------------- variables globales --------------------------------- */


estadoRecepcion_t estadoRecepcion;
colaCeldaPool_t celdasPools;

// inicializo id de paquete en cero como variable global
uint32_t idPaqueteAutonum = 0;


/* ---------------------------- implementacion de funciones --------------------------------- */

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.

 */
char* itoa(int value, char* result, int base) {
   // check that the base if valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}

/**
 * @fn static int32_t inicializarColaPool(colaPool_t*colaPool)
 *
 * @brief inicializo las colas de pools
 *
 */

static int32_t inicializarColaCeldaPool(colaCeldaPool_t*colaCeldaPool) {

  colaCeldaPool->ini = 0;
  colaCeldaPool->fin = 0;
  colaCeldaPool->estadoColaCeldaPool = COLA_CELDA_POOL_VACIO;
  return 0;

}
/**
 * @fn static int32_t obtenerCeldaPoolParaEscribir(colaPool_t*colaPool)
 *
 * @brief inicializo las colas de pools
 *
 */

static poolInfo_t* obtenerCeldaPoolParaEscribir(colaCeldaPool_t*colaCeldaPool) {

  poolInfo_t*poolObtenido;

  // si la cola de celdas esta llena, retorno null
  if(colaCeldaPool->estadoColaCeldaPool == COLA_CELDA_POOL_LLENO) {
      return NULL;
  }
  poolObtenido = &(colaCeldaPool->poolsAbiertos[colaCeldaPool->fin]);

  colaCeldaPool->fin++;

  if(colaCeldaPool->fin >= POOLS_MAX)
    colaCeldaPool->fin = 0;

  if(colaCeldaPool->fin == colaCeldaPool->ini)
    colaCeldaPool->estadoColaCeldaPool = COLA_CELDA_POOL_LLENO;
  else
    colaCeldaPool->estadoColaCeldaPool = COLA_CELDA_POOL_NORMAL;

  return poolObtenido;
}

/**
 * @fn static int32_t obtenerCeldaPoolParaLiberar(colaPool_t*colaPool)
 *
 * @brief obtengo la celda mas antigua
 *
 */

static poolInfo_t* obtenerCeldaPoolParaLiberar(colaCeldaPool_t*colaCeldaPool) {

  poolInfo_t*poolObtenido;

  // si la cola de celdas esta llena, retorno null
  if(colaCeldaPool->estadoColaCeldaPool == COLA_CELDA_POOL_VACIO) {
      return NULL;
  }
  poolObtenido = &(colaCeldaPool->poolsAbiertos[colaCeldaPool->ini]);

  colaCeldaPool->ini++;

  if(colaCeldaPool->ini >= POOLS_MAX)
    colaCeldaPool->ini = 0;

  if(colaCeldaPool->fin == colaCeldaPool->ini)
    colaCeldaPool->estadoColaCeldaPool = COLA_CELDA_POOL_VACIO;
  else
    colaCeldaPool->estadoColaCeldaPool = COLA_CELDA_POOL_NORMAL;

  return poolObtenido;
}

/**
 * @fn static int32_t obtenerCeldaPoolParaDescartar(colaPool_t*colaPool)
 *
 * @brief obtengo la celda mas reciente para descartar
 *
 */

static poolInfo_t* obtenerCeldaPoolParaDescartar(colaCeldaPool_t*colaCeldaPool) {

  poolInfo_t*poolObtenido;

  // si la cola de celdas esta llena, retorno null
  if(colaCeldaPool->estadoColaCeldaPool == COLA_CELDA_POOL_VACIO) {
      return NULL;
  }

  if(colaCeldaPool->fin == 0)
    colaCeldaPool->fin = POOLS_MAX - 1;
  else
    colaCeldaPool->fin--;

  poolObtenido = &(colaCeldaPool->poolsAbiertos[colaCeldaPool->fin]);

  if(colaCeldaPool->fin == colaCeldaPool->ini)
    colaCeldaPool->estadoColaCeldaPool = COLA_CELDA_POOL_VACIO;
  else
    colaCeldaPool->estadoColaCeldaPool = COLA_CELDA_POOL_NORMAL;

  return poolObtenido;
}



/**
 * @fn int32_t inicializarQueues (void)
 *
 * @brief inicializo las colas de datos
 *
 */
int32_t inicializarQueues (void) {

  queMayusculizar = xQueueCreate (QUEUE_MAYUSCULIZAR_L, sizeof(poolInfo_t*));
  queMinusculizar = xQueueCreate (QUEUE_MINUSCULIZAR_L, sizeof(poolInfo_t*));

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

  QMPool_init(&memPoolToken, poolMemoriaToken, sizeof(poolMemoriaToken), POOL_MEMORIA_TOKEN_TBLOQUE);

  inicializarColaCeldaPool(&celdasPools);

  return 0;
}

/**
 * @fn void tareaMayusculizar (void*taskPtr)
 *
 * @brief tarea que espera bloqueada una queue para pasar caracteres en minus a mayus y
 *        ponerlo en una cola de salida
 */

void tareaMayusculizar (void*taskPtr) {

  poolInfo_t*itemQueue;
  uint8_t i;
  UBaseType_t stackMedido;
  token_t tokenR;

  uint8_t bufStack[PRT_TAM_PAQ_REPSTACK];

  while( TRUE ) {

      xQueueReceive(queMayusculizar, &itemQueue, portMAX_DELAY);
      itemQueue->mperf.tiempo_de_inicio = MEDIR_TIEMPO();
      for(i = PRT_DAT_INI_I; i < (itemQueue->bufL - 1); i++ )
          if( (itemQueue->buf[i] >= 'a') && (itemQueue->buf[i] <= 'z') )
            itemQueue->buf[i] -= 'a' - 'A';

      itemQueue->mperf.tiempo_de_fin = MEDIR_TIEMPO();
      // itemQueue->token->tiempo_de_salida = MEDIR_TIEMPO();

      xQueueSend(uartPC.queTransmision, itemQueue->token, portMAX_DELAY);
      // pongo token en un valor no válido
      tokenR.id_de_paquete = 0xFFFFFFFF;
      // insisto mientras que el token recibido no sea para mí
      while(tokenR.id_de_paquete != itemQueue->token->id_de_paquete) {
        xQueuePeek(uartPC.queTokenACT, &tokenR, portMAX_DELAY);
        // mido tiempo fin de transmision
        // itemQueue->token->tiempo_de_transmision = MEDIR_TIEMPO();
      }
      // cuando salí, es porque el token es para mí
      xQueueReceive(uartPC.queTokenACT, &tokenR, portMAX_DELAY);

      // asigno el puntero payload al buffer local
      itemQueue->token->payload = bufStack;
      // mido maximo de stack, armo paquete y tiro
      stackMedido = uxTaskGetStackHighWaterMark(NULL);
      // sobreescribo el mismo token que antes
      itemQueue->token->largo_del_paquete = armarPaqueteMedicionStack(itemQueue->token->payload, stackMedido);
      // mando info de stack
      xQueueSend(uartPC.queTransmision, itemQueue->token, portMAX_DELAY);

      // pongo token en un valor no válido
      tokenR.id_de_paquete= 0xFFFFFFFF;
      // insisto mientras que el token recibido no sea para mí
      while(tokenR.id_de_paquete != itemQueue->token->id_de_paquete) {
        xQueuePeek(uartPC.queTokenACT, &tokenR, portMAX_DELAY);
      }
      // cuando salí, es porque el token es para mí
      xQueueReceive(uartPC.queTokenACT, &tokenR, portMAX_DELAY);

      // una vez que procese los datos, libero el pool recibido
      liberarPoolMasAntiguo();

  }

}



/**
 * @fn void tareaMinusculizar (void*taskPtr)
 *
 * @brief tarea que espera bloqueada una queue para pasar caracteres en mayus a minus y
 *        ponerlo en una cola de salida
 */

void tareaMinusculizar (void*taskPtr) {

  poolInfo_t*itemQueue;
  uint8_t i;
  UBaseType_t stackMedido;
  token_t tokenR;

  uint8_t bufStack[PRT_TAM_PAQ_REPSTACK];

  while( TRUE ) {

      xQueueReceive(queMinusculizar, &itemQueue, portMAX_DELAY);
      itemQueue->mperf.tiempo_de_inicio = MEDIR_TIEMPO();
      for(i = PRT_DAT_INI_I; i < (itemQueue->bufL - 1); i++ )
          if( (itemQueue->buf[i] >= 'A') && (itemQueue->buf[i] <= 'Z') )
            itemQueue->buf[i] += 'a' - 'A';

      itemQueue->mperf.tiempo_de_fin = MEDIR_TIEMPO();
            // itemQueue->token->tiempo_de_salida = MEDIR_TIEMPO();

      xQueueSend(uartPC.queTransmision, itemQueue->token, portMAX_DELAY);
      // pongo token en un valor no válido
      tokenR.id_de_paquete = 0xFFFFFFFF;
      // insisto mientras que el token recibido no sea para mí
      while(tokenR.id_de_paquete != itemQueue->token->id_de_paquete) {
        xQueuePeek(uartPC.queTokenACT, &tokenR, portMAX_DELAY);
        // mido tiempo fin de transmision
        // itemQueue->token->tiempo_de_transmision = MEDIR_TIEMPO();
      }
      // cuando salí, es porque el token es para mí
      xQueueReceive(uartPC.queTokenACT, &tokenR, portMAX_DELAY);

      // asigno el puntero payload al buffer local
      itemQueue->token->payload = bufStack;
      // mido maximo de stack, armo paquete y tiro
      stackMedido = uxTaskGetStackHighWaterMark(NULL);
      // sobreescribo el mismo token que antes
      itemQueue->token->largo_del_paquete = armarPaqueteMedicionStack(itemQueue->token->payload, stackMedido);
      // mando info de stack
      xQueueSend(uartPC.queTransmision, itemQueue->token, portMAX_DELAY);

      // pongo token en un valor no válido
      tokenR.id_de_paquete= 0xFFFFFFFF;
      // insisto mientras que el token recibido no sea para mí
      while(tokenR.id_de_paquete != itemQueue->token->id_de_paquete) {
        xQueuePeek(uartPC.queTokenACT, &tokenR, portMAX_DELAY);
      }
      // cuando salí, es porque el token es para mí
      xQueueReceive(uartPC.queTokenACT, &tokenR, portMAX_DELAY);

      // una vez que procese los datos, libero el pool recibido
      liberarPoolMasAntiguo();

  }

}

/**
 * @fn void tareaMedirPerformance (void*taskPtr)
 *
 * @brief tarea que espera bloqueada una queue para pasar caracteres en minus a mayus y
 *        ponerlo en una cola de salida
 */

void tareaMedirPerformance (void*taskPtr) {

  poolInfo_t*itemQueue;
  uint8_t i;
  UBaseType_t stackMedido;
  token_t tokenR;

  uint8_t bufPerformance[sizeof(performance_t) + PRT_TAM_PAQ_REPSTACK];


  while( TRUE ) {

    xQueueReceive(queMedirPerformance, &itemQueue, portMAX_DELAY);
    itemQueue->mperf.tiempo_de_inicio = MEDIR_TIEMPO();
    for(i = PRT_DAT_INI_I; i < (itemQueue->bufL - 1); i++ )
        if( (itemQueue->buf[i] >= 'a') && (itemQueue->buf[i] <= 'z') )
          itemQueue->buf[i] -= 'a' - 'A';

    itemQueue->mperf.tiempo_de_fin = MEDIR_TIEMPO();

    xQueueSend(uartPC.queTransmision, itemQueue->token, portMAX_DELAY);
    // pongo token en un valor no válido
    tokenR.id_de_paquete = 0xFFFFFFFF;
    // insisto mientras que el token recibido no sea para mí
    while(tokenR.id_de_paquete != itemQueue->token->id_de_paquete) {
      xQueuePeek(uartPC.queTokenACT, &tokenR, portMAX_DELAY);
      // mido tiempo fin de transmision
      itemQueue->mperf.tiempo_de_transmision = MEDIR_TIEMPO();
    }
    // cuando salí, es porque el token es para mí
    xQueueReceive(uartPC.queTokenACT, &tokenR, portMAX_DELAY);




    // una vez que procese los datos, libero el pool recibido
    liberarPoolMasAntiguo();

  }

}


/**
 * @fn void armarPaqueteMedicionStack (uint8_t*buf)
 *
 * @brief paquete que mide el máximo historico del stack y arma el paquete
 *
 */



uint8_t armarPaqueteMedicionStack (uint8_t*buf, UBaseType_t stackMedido) {

  uint8_t stackStringLargo, i;

  char stackString[STACK_STRING_L];

  itoa((int32_t) stackMedido, stackString, BASE_STACK);

  stackStringLargo = strlen(stackString);

  buf[PRT_STX_I] = PRT_STX;
  buf[PRT_OP_I] = PRT_REPSTACK;
  buf[PRT_TAM_I] = (uint8_t) stackStringLargo;

  for(i = 0; i < stackStringLargo; i++)
      buf[PRT_DAT_INI_I+i] = stackString[i];

  buf[PRT_DAT_INI_I + stackStringLargo] = PRT_ETX;

  return (PRT_BYTES_PROTCOLO + stackStringLargo);

}

/**
 * @fn void armarPaqueteMedicionHeap (uint8_t*buf)
 *
 * @brief paquete que mide el máximo historico del stack y arma el paquete
 *
 */



static uint8_t armarPaqueteMedicionHeap (uint8_t*buf) {

  uint8_t heapStringLargo, i;

  uint32_t heapMedido;

  char heapString[HEAP_STRING_L];


  heapMedido = POOL_MEMORIA_S_T + POOL_MEMORIA_M_T + POOL_MEMORIA_L_T;


  itoa((int32_t) heapMedido, heapString, BASE_HEAP);

  heapStringLargo = strlen(heapString);

  buf[PRT_STX_I] = PRT_STX;
  buf[PRT_OP_I] = PRT_REPHEAP;
  buf[PRT_TAM_I] = (uint8_t) heapStringLargo;

  for(i = 0; i < heapStringLargo; i++)
      buf[PRT_DAT_INI_I+i] = heapString[i];

  buf[PRT_DAT_INI_I + heapStringLargo] = PRT_ETX;

  return (PRT_BYTES_PROTCOLO + heapStringLargo);

}

/**
 * @fn void tareaRecibirPaquete (void* taskParam)
 *
 * @brief tarea con máquina de estados para procesar el paquete
 *
 */
void tareaRecibirPaquete (void* taskParam) {


  uint32_t tiempo_de_llegada_temp;       // tiempo que se recibió el primer byte de paquete (STX) temporal hasta reservar pool
  uint32_t tiempo_de_recepcion_temp;     // tiempo que se recibió el último byte de paquete (ETX) temporal hasta reservar el pool
  // variable temporal del byte recibido
  uint8_t byteRecibido;
  // contador de bytes recibidos y tamaño total
  uint8_t cuentaT = 0;
  uint8_t tam;

  // punteros a pool y estructura de control
  uint8_t*pDatos;
  poolInfo_t*poolObtenido;
  // operacion recibida
  op_t operacion;
  token_t token, tokenR;
  uint8_t bufHeap[HEAP_STRING_L], largoBufHeap;

  xQueueReceive(uartPC.queueRxUART, &byteRecibido, TIMEOUT_RX_PAQUETE);

  // espero recibir una 'A' para mandar el heap y comenzar el programa
  if(byteRecibido == 'A') {
    largoBufHeap = armarPaqueteMedicionHeap(bufHeap);
    token.id_de_paquete = 0x00000001;
    token.payload = bufHeap;
    token.largo_del_paquete = largoBufHeap;

    xQueueSend(uartPC.queTransmision, &token, portMAX_DELAY);

    xQueuePeek(uartPC.queTokenACT, &tokenR, portMAX_DELAY);
    if(tokenR.id_de_paquete == token.id_de_paquete)
      xQueueReceive(uartPC.queTokenACT, &tokenR, portMAX_DELAY);
    // descargarBufferEnFIFOUARTTx(uartPC.perif, bufHeap, largoBufHeap);

  }

  while(TRUE) {

    xQueueReceive(uartPC.queueRxUART, &byteRecibido, TIMEOUT_RX_PAQUETE);

    switch(estadoRecepcion) {

      case RECIBIR_STX:

        if(byteRecibido == PRT_STX) {
            estadoRecepcion = RECIBIR_OP;
            tiempo_de_llegada_temp = MEDIR_TIEMPO();
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
        poolObtenido = obtenerPoolPorTam(tam);
        // no hay mas memoria libre
        // if(poolObtenido == NULL)
          // configASSERT();

        // me hago una copia del puntero para guardar el inicio
        pDatos = poolObtenido->buf;

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
          tiempo_de_recepcion_temp = MEDIR_TIEMPO();
          *pDatos = PRT_ETX;

          // guardo los tiempos que ya tengo medidos recien y valores que ya conozco:
          poolObtenido->mperf.tiempo_de_llegada = tiempo_de_llegada_temp;
          poolObtenido->mperf.memoria_alojada = poolObtenido->tPool;
          poolObtenido->mperf.tiempo_de_recepcion = tiempo_de_recepcion_temp;

          poolObtenido->token->id_de_paquete = idPaqueteAutonum;
          poolObtenido->token->payload = poolObtenido->buf;
          poolObtenido->token->largo_del_paquete = tam + PRT_BYTES_PROTCOLO;

          idPaqueteAutonum++;

          procesarDatos(poolObtenido, operacion);

        }
        else
          liberarPoolMasReciente();
          // si no recibí ETX, descarto los datos recibidos
          // esta funcion no libera pool de token porque no llega a asignarse.


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
    case PRT_PERF:
      return op;
    default:
      return -1;
  }

}


/**
 * @fn uint32_t clasificarTamPool (poolInfo_t*pool)
 *
 * @brief libero pool mas reciente
 */

static uint32_t liberarPoolMasReciente (void) {

    poolInfo_t*poolObtenido;

    // asigno el casillero al puntero
    poolObtenido = obtenerCeldaPoolParaDescartar(&celdasPools);

    // pregunto si pude obtener un pool abierto
    if(poolObtenido == NULL)
      while(1);

    // libero pool asociado a token
    QMPool_put(&memPoolToken, poolObtenido->token);
    // libero pool asociado al paquete
    QMPool_put(poolObtenido->ctrlPool, poolObtenido->buf);

    return 0;
}
/**
 * @fn uint32_t liberarPoolMasAntiguo (poolInfo_t*pool)
 *
 * @brief libero pool mas antiguo
 */

uint32_t liberarPoolMasAntiguo (void) {

    poolInfo_t*poolObtenido;

    // asigno el casillero al puntero
    poolObtenido = obtenerCeldaPoolParaLiberar(&celdasPools);

    // pregunto si pude obtener un pool abierto
    if(poolObtenido == NULL)
      while(1);

    // libero pool asociado a token
    QMPool_put(&memPoolToken, poolObtenido->token);
    // libero pool asociado a paquete
    QMPool_put(poolObtenido->ctrlPool, poolObtenido->buf);

    return 0;
}

/**
 * @fn uint8_t* obtenerPoolPorTam (uint8_t tam)
 *
 * @brief recibo el tamaño del buffer a procesar y devuelvo un pool de tamaño adecuado
 */

static poolInfo_t* obtenerPoolPorTam (uint8_t tam) {

  poolInfo_t*poolObtenido;

  // asigno el casillero al puntero
  poolObtenido = obtenerCeldaPoolParaEscribir(&celdasPools);

  if(poolObtenido == NULL)
    return NULL;

  // al tam recibido, le sumo 4 bytes por el STX, OP, TAM y ETX
  tam = tam + PRT_BYTES_PROTCOLO;

  if(tam <= POOL_MEMORIA_S_TBLOQUE) {
    poolObtenido->buf = QMPool_get(&memPoolS, 0U);
    poolObtenido->bufL = tam;
    poolObtenido->ctrlPool = &memPoolS;
    poolObtenido->tPool = chico;
  }
  else if((tam > POOL_MEMORIA_S_TBLOQUE) && (tam <= POOL_MEMORIA_M_TBLOQUE)) {
    poolObtenido->buf = QMPool_get(&memPoolM, 0U);
    poolObtenido->bufL = tam;
    poolObtenido->ctrlPool = &memPoolM;
    poolObtenido->tPool = medio;
  }
  else if(tam > POOL_MEMORIA_M_TBLOQUE) {
    poolObtenido->buf = QMPool_get(&memPoolL, 0U);
    poolObtenido->bufL = tam;
    poolObtenido->ctrlPool = &memPoolL;
    poolObtenido->tPool = grande;
  }

  // reservo un lugar en el pool de tokens y copio el puntero
  poolObtenido->token = QMPool_get(&memPoolToken, 0UL);

  return poolObtenido;
}

/**
 * @fn int32_t procesarDatos(uint8_t*pPool, uint8_t tam, op_t op);
 *
 * @brief recibo el pool y su longitud, la operación, y proceso el texto
 */

static int32_t procesarDatos(poolInfo_t*poolAsociado, op_t op) {

  switch(op){
    case PRT_MAYUS:
      xQueueSend(queMayusculizar, &poolAsociado, portMAX_DELAY);
      break;
    case PRT_MINUS:
      xQueueSend(queMinusculizar, &poolAsociado, portMAX_DELAY);
      break;
    case PRT_REPSTACK:
      break;
    case PRT_REPHEAP:
      break;
    case PRT_MSJEST:
      break;
    case PRT_PERF:
      xQueueSend(queMedirPerformance, &poolAsociado, portMAX_DELAY);
      break;
    default:
      return -1;
  }

}


