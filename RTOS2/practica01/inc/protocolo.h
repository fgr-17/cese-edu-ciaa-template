/*
 * protocolo.h
 *
 *  Created on: 18 sep. 2018
 *      Author: froux
 */

#ifndef RTOS2_PRACTICA01_INC_PROTOCOLO_H_
#define RTOS2_PRACTICA01_INC_PROTOCOLO_H_

/* ---------------------------- defines --------------------------------- */

#define TIMEOUT_RX_PAQUETE portMAX_DELAY

// #define PROTOCOLO_DEBUG

#ifndef PROTOCOLO_DEBUG
  #define PRT_STX             0x55
  #define PRT_ETX             0xAA
  typedef enum {PRT_MAYUS = 0x00, PRT_MINUS = 0x01, PRT_REPSTACK = 0x02, PRT_REPHEAP = 0x03, PRT_MSJEST = 0x04} op_t;
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

#define PRT_BYTES_PROTCOLO  4

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

#define  QUEUE_PROC_L                   4

#define QUEUE_MAYUSCULIZAR_L            QUEUE_PROC_L
#define QUEUE_MINUSCULIZAR_L            QUEUE_PROC_L
#define QUEUE_MAYUSCULIZADOS_L          QUEUE_PROC_L
#define QUEUE_MINUSCULIZADOS_L          QUEUE_PROC_L


/* ---------------------------- tipos de dato --------------------------------- */

typedef enum {RECIBIR_STX, RECIBIR_OP, RECIBIR_T, RECIBIENDO_DATOS, RECIBIR_ETX} estadoRecepcion_t;
typedef enum {chico, medio, grande} tPool_t;

typedef enum {COLA_CELDA_POOL_LLENO, COLA_CELDA_POOL_VACIO, COLA_CELDA_POOL_NORMAL} estadoColaPool_t;


typedef struct {

  uint8_t*buf;
  uint8_t bufL;
  tPool_t tPool;
  QMPool*ctrlPool;
  uint8_t indice;
} poolInfo_t;

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


#endif /* RTOS2_PRACTICA01_INC_PROTOCOLO_H_ */
