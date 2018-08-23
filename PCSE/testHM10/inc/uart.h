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
} uartQueue_t;


typedef struct {
  uartMap_t perif;
  uint32_t baudrate;
  QueueHandle_t queueUART;
}uart_t;

/* ---------------------------- prototipos --------------------------------- */

extern int32_t inicializarTareaEnviarDatosUARTs ( void );
void tareaEnviarDatosUART ( void* uartN );


/* ---------------------------- variables --------------------------------- */

extern uart_t uartPC;
extern uart_t uartBLE;

#endif /* RTOS1_EJERCICIO0602_INC_UART_H_ */
