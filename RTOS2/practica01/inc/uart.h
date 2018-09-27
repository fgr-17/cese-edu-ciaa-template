/*
 * rtos.h
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
 */

#ifndef RTOS1_EJERCICIO0602_INC_UART_H_
#define RTOS1_EJERCICIO0602_INC_UART_H_

// largo del mensaje del struct para la queue
#define MENSAJE_L                           255
// largo de la queue
#define QUEUE_UART_L                        5
// maximo delay entre dos caracteres en ms
#define UART_DELAY_RX_ENTRE_CARACTERES      4
// largo de la fifo de la UART
#define FIFO_UART_L                         16




#define UART_PC                     UART_USB
#define UART_PC_BAUDRATE            115200

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

  // mutex para controlar la salida de datos
  SemaphoreHandle_t tx_thre;


  uartModo_t modo;
}uart_t;


/** @brief union para convertir bytes<->words */
typedef union {

  uint16_t word;
  uint8_t bytes[2];

} uartWord_Byte_t;

/** @brief union para convertir bytes<->words */
typedef union {

  uint32_t dword;
  uint16_t word[2];
  uint8_t bytes[4];

} uartDword_Byte_t;


/* ---------------------------- prototipos --------------------------------- */


extern int32_t inicializarStructUart (uart_t*uart, uartMap_t perif, uint32_t baudrate);
extern int32_t inicializarTareaEnviarDatosUARTs ( void );

extern int32_t configurarUARTModoBytes ( uart_t*uartN);

extern void tareaEnviarDatosUART ( void* uartN );
extern void tareaRecibirStringPorTimeout (void* uartN);


extern int32_t descargarBufferEnFIFOUARTTx (uartMap_t uartPerif, const char* buf, uint8_t n);

/* ---------------------------- variables --------------------------------- */

extern uart_t uartPC;
extern uart_t uartBLE;

#endif /* RTOS1_EJERCICIO0602_INC_UART_H_ */
