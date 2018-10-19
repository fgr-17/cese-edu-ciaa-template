/*
 * protocolo.h
 *
 *  Created on: 18 sep. 2018
 *      Author: froux
 */

#ifndef RTOS2_PRACTICA01_INC_PROTOCOLO_H_
#define RTOS2_PRACTICA01_INC_PROTOCOLO_H_

/* ---------------------------- includes --------------------------------- */


/* ---------------------------- defines --------------------------------- */

#define TIMEOUT_RX_PAQUETE portMAX_DELAY

// #define PROTOCOLO_DEBUG

#ifndef PROTOCOLO_DEBUG
  #define PRT_STX             0x55
  #define PRT_ETX             0xAA
  typedef enum {PRT_MAYUS,PRT_MINUS, PRT_REPSTACK, PRT_REPHEAP, PRT_MSJEST, PRT_PERF} op_t;
#else
  typedef enum {PRT_MAYUS = 'q', PRT_MINUS = 'w', PRT_REPSTACK = 'e', PRT_REPHEAP = 'r', PRT_MSJEST = 't'} op_t;
  #define PRT_STX             'a'
  #define PRT_ETX             'z'
#endif


// indice byte de STX
#define PRT_STX_I           0
// indice byte de OP
#define PRT_OP_I            1
// indice byte de TAM
#define PRT_TAM_I           2
// indice byte de inicio de datos
#define PRT_DAT_INI_I       3

/** @brief cantidad de bytes fijos de protocolo */
#define PRT_BYTES_PROTCOLO  4

// defines para el manejo del string con el valor de heap
#define HEAP_STRING_L       8
#define BASE_HEAP           10
// defines para el manejo del string con el valor de stack
#define STACK_STRING_L      6
#define BASE_STACK          10

/** @brief cantidad de bytes que mando cuando informo el stack */
#define PRT_TAM_PAQ_REPSTACK              (PRT_BYTES_PROTCOLO + STACK_STRING_L)



// cantidad total de pools que pueden abrirse en simultaneo
#define POOLS_MAX   (POOL_MEMORIA_S_CBLOQUES + POOL_MEMORIA_M_CBLOQUES + POOL_MEMORIA_L_CBLOQUES)
// #define POOLS_MAX   3
#define POOL_MEMORIA_S_TBLOQUE          16
#define POOL_MEMORIA_S_CBLOQUES         8
#define POOL_MEMORIA_S_T                (POOL_MEMORIA_S_TBLOQUE * POOL_MEMORIA_S_CBLOQUES)

#define POOL_MEMORIA_M_TBLOQUE          64
#define POOL_MEMORIA_M_CBLOQUES         4
#define POOL_MEMORIA_M_T                (POOL_MEMORIA_M_TBLOQUE * POOL_MEMORIA_M_CBLOQUES)

#define POOL_MEMORIA_L_TBLOQUE          256
#define POOL_MEMORIA_L_CBLOQUES         2
#define POOL_MEMORIA_L_T                (POOL_MEMORIA_L_TBLOQUE * POOL_MEMORIA_L_CBLOQUES)

/** @brief defines para el pool del token */
#define POOL_MEMORIA_TOKEN_TBLOQUE          sizeof(token_t)
#define POOL_MEMORIA_TOKEN_CBLOQUES         POOLS_MAX
#define POOL_MEMORIA_TOKEN_T                (POOL_MEMORIA_TOKEN_TBLOQUE * POOL_MEMORIA_TOKEN_CBLOQUES)

/** @brief defines para el manejo de colas de datos */

#define  QUEUE_PROC_L                   4

#define QUEUE_MAYUSCULIZAR_L            QUEUE_PROC_L
#define QUEUE_MINUSCULIZAR_L            QUEUE_PROC_L
#define QUEUE_TRANSMISION_L             QUEUE_PROC_L


/** @brief macro para medir tiempo con resolucion 1ms (tick)*/
#define MEDIR_TIEMPO()      xTaskGetTickCount()


/* ---------------------------- tipos de dato --------------------------------- */

typedef enum {RECIBIR_STX, RECIBIR_OP, RECIBIR_T, RECIBIENDO_DATOS, RECIBIR_ETX} estadoRecepcion_t;
typedef enum {chico = POOL_MEMORIA_S_TBLOQUE, medio = POOL_MEMORIA_M_TBLOQUE, grande = POOL_MEMORIA_L_TBLOQUE} tPool_t;

typedef enum {COLA_CELDA_POOL_LLENO, COLA_CELDA_POOL_VACIO, COLA_CELDA_POOL_NORMAL} estadoColaPool_t;

/**
 * @brief campos de inicio del paquete
 */
typedef struct {

  uint8_t stx_byte;
  uint8_t op_byte;
  uint8_t tam_byte;

}paqCampos_t;


/** @brief estructura de token */

typedef struct {
  uint32_t id_de_paquete;           // identificacion de tránsito de paq. por el sistema. Autonumérico
  uint8_t* payload;                 // puntero a paquete de datos a procesar
  uint32_t tiempo_de_llegada;       // tiempo que se recibió el primer byte de paquete (STX)
  uint32_t tiempo_de_recepcion;     // tiempo que se recibió el último byte de paquete (ETX)
  uint32_t tiempo_de_inicio;        // tiempo que se extrae el puntero al paquete en la tarea mayusculizar o minusculizar
  uint32_t tiempo_de_fin;           // tiempo que se pone el puntero a paquete en la tarea queMayusculizar
  uint32_t tiempo_de_salida;        // tiempo que se transmitió el primer byte de paquete (STX)
  uint32_t tiempo_de_transmision;   // tiempo que se transmitió el ultimo byte de paquete (ETX)
  uint16_t largo_del_paquete;       // largo total del paquete recibido (largo del dato MAS largo del header)
  uint16_t memoria_alojada;         // tamaño del bloque extraido del pool de memoria del sistema para el bloque
} token_t;


/**
 * @brief campos para manejo del pool de memoria
 */

typedef struct {
  uint8_t*buf;                      // puntero al inicio del pool
  token_t*token;                    // puntero al token
  uint8_t bufL;                     // cantidad de datos válidos
  tPool_t tPool;                    // tamaño del pool
  QMPool*ctrlPool;                  // estructura de control

  uint8_t indice;                   // indice de recorrido
} poolInfo_t;


/**
 * @brief cola de datos para el manejo de pools.
 */

typedef struct {
  poolInfo_t poolsAbiertos[POOLS_MAX];
  uint8_t ini;
  uint8_t fin;
  estadoColaPool_t estadoColaCeldaPool;
} colaCeldaPool_t;

/* ---------------------------- funciones externas --------------------------------- */

extern int32_t inicializarRecibirPaquete (void);

extern void tareaMayusculizar (void*taskPtr);
extern void tareaEnviarMayusculizados (void*taskPtr);

extern void tareaMinusculizar (void*taskPtr);
extern void tareaEnviarMinusculizados (void*taskPtr);


extern void tareaRecibirPaquete (void* taskParam);

extern uint32_t liberarPoolMasAntiguo (void);
extern uint8_t armarPaqueteMedicionStack (uint8_t*buf, UBaseType_t stackMedido);

/* ---------------------------- variables globales --------------------------------- */
extern QueueHandle_t queTransmision;


#endif /* RTOS2_PRACTICA01_INC_PROTOCOLO_H_ */
