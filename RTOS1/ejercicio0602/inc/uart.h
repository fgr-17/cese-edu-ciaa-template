/*
 * rtos.h
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
 */

#ifndef RTOS1_EJERCICIO0602_INC_UART_H_
#define RTOS1_EJERCICIO0602_INC_UART_H_

// largo del mensaje del struct para la queue
#define MENSAJE_L 50
// largo de la queue
#define QUEUE_UART_L  5

/* ---------------------------- tipo uart --------------------------------- */

typedef struct {

  char mensaje[MENSAJE_L];

} uart_t;

/* ---------------------------- prototipos --------------------------------- */

extern void tareaEnviarDatosUART ( void* taskParmPtr );

/* ---------------------------- variables --------------------------------- */


extern SemaphoreHandle_t semaforoUART;
extern QueueHandle_t queueUART;

#endif /* RTOS1_EJERCICIO0602_INC_UART_H_ */
