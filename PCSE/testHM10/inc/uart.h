/*
 * rtos.h
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
 */

#ifndef RTOS1_EJERCICIO0602_INC_UART_H_
#define RTOS1_EJERCICIO0602_INC_UART_H_

// largo del mensaje del struct para la queue
#define MENSAJE_L 100
// largo de la queue
#define QUEUE_UART_L  5
// maximo delay entre dos caracteres en ms
#define UART_DELAY_RX_ENTRE_CARACTERES      4
// largo de la fifo de la UART
#define FIFO_UART_L       16

/* ---------------------------- tipo uart --------------------------------- */

/** @brief modos de funcionamiento de la irq de la uart */
typedef enum {MODO_BYTES, MODO_UMBRAL} uartModo_t;

/** @brief item de la cola de datos para la uart */
typedef struct {
  char mensaje[MENSAJE_L];
} uartQueue_t;

/** @brief  tipo de dato para el manejo general de uart. */
typedef struct {
  uartMap_t perif;
  uint32_t baudrate;
  QueueHandle_t queueTxUART;
  QueueHandle_t queueRxUART;
  QueueHandle_t queueRxStringUART;
  SemaphoreHandle_t semaphoreInicioIRQ;
  uartModo_t modo;
}uart_t;

/* ---------------------------- prototipos --------------------------------- */


extern char* itoa(int value, char* result, int base);
extern int32_t inicializarStructUart (uart_t*uart, uartMap_t perif, uint32_t baudrate);
extern int32_t inicializarTareaEnviarDatosUARTs ( void );

extern int32_t configurarUARTModoUmbral ( uart_t*uartN);
extern int32_t configurarUARTModoBytes ( uart_t*uartN);


extern void tareaEnviarDatosUART ( void* uartN );
extern void tareaRecibirStringPorTimeout (void* uartN);


/* ---------------------------- variables --------------------------------- */

extern uart_t uartPC;
extern uart_t uartBLE;

#endif /* RTOS1_EJERCICIO0602_INC_UART_H_ */
