/*
 * gpio.h
 *
 *  Created on: 30 jul. 2018
 *      Author: froux
 */

#ifndef RTOS1_CLASE5_INC_GPIO_H_
#define RTOS1_CLASE5_INC_GPIO_H_

/* ---------------------- defines -------------------------- */

#define TECLA1_CANAL_DESC     0
#define TECLA1_CANAL_ASC      0
#define TECLA1_PORT           0
#define TECLA1_PIN            4

#define TECLA2_CANAL_DESC     1
#define TECLA2_CANAL_ASC      1
#define TECLA2_PORT           0
#define TECLA2_PIN            8


/* Largo de las queues de teclas */
#define TECLAS_QUEUE_L    4


/* ---------------------- tipos de dato -------------------------- */

/* enum para indice de tecla */
// typedef enum {TEC1_IND, TEC2_IND, TEC3_IND, TEC4_IND} tecInd_t;
typedef enum {FLANCO_SUBIDA, FLANCO_BAJADA} tecFlanco_t;

typedef struct {

  gpioMap_t teclaPresionada;
  TickType_t tickEvento;
  tecFlanco_t flancoDetectado;

} tecQueue_t;

/* ---------------------- funciones globales -------------------------- */

extern int inicializarTecla (void);
extern int inicializarQueuesTeclas ( void );
/* ---------------------- variables globales -------------------------- */

extern QueueHandle_t teclasQueue;

#endif /* RTOS1_CLASE5_INC_GPIO_H_ */
