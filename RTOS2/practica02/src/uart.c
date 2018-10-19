/**
 * @file uart.c
 *
 * @date 17 ago. 2018
 * @author froux
 */

/* ---------------------------- includes --------------------------------- */

#include "stdint.h"
#include "string.h"

// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sapi.h"        // <= Biblioteca sAPI
#include "uart.h"
#include "qmpool.h"
#include "protocolo.h"
/* ---------------------------- prototipos --------------------------------- */

int32_t inicializarStructUart (uart_t*uart, uartMap_t perif, uint32_t baudrate);
int32_t inicializarTareaEnviarDatosUARTs ( void );

int32_t configurarUARTModoBytes ( uart_t*uartN);

int32_t descargarBufferEnFIFOUARTTx (uartMap_t uartPerif, const uint8_t* buf, uint8_t n);

/* ---------------------------- variables globales --------------------------------- */

uart_t uartPC;
uartQueue_t itemColaUART;
/* ---------------------------- implementacion --------------------------------- */

/**
 * @fn int32_t inicializarTareaEnviarDatosUART ( void ) {
 *
 * @brief inicializo colas de datos de las teclas
 *
 */

int32_t inicializarStructUart (uart_t*uart, uartMap_t perif, uint32_t baudrate) {

  uart->perif = perif;
  uart->baudrate = baudrate;
  // la cola de datos de recepcion recibe de a bytes
  uart->queueRxUART = xQueueCreate (QUEUE_UART_L, sizeof(char));
  // creo semaforo binario para manejar la irq de tx de la uart
  uart->tx_thre = xSemaphoreCreateBinary();
  // arranco con el semaforo libre.
  xSemaphoreGive(uart->tx_thre);
  // cola de datos para devolver id
  uart->queTokenACT = xQueueCreate( QUEUE_UART_L, sizeof(uint32_t));
  return 0;
}

/**
 * @fn int32_t inicializarTareaEnviarDatosUART ( void ) {
 *
 * @brief Inicializo UARTS, colas de datos de cada una e IRQ
 */

int32_t inicializarTareaEnviarDatosUARTs ( void ) {

  inicializarStructUart(&uartPC, UART_PC, UART_PC_BAUDRATE);
  uartInit( uartPC.perif, uartPC.baudrate);
  uartRxInterruptSet(UART_PC, TRUE);
  uartTxInterruptSet(UART_PC, TRUE);
  return 0;
}

/**
 * @fn int32_t configurarUARTModoUmbral ( uart_t*uartN)
 *
 * @brief entro a modo umbral
 */

int32_t configurarUARTModoBytes ( uart_t*uartN) {

  taskENTER_CRITICAL();
  // Deshabilito la IRQ
  //uartRxInterruptSet(uartN->perif, FALSE);
  // SETEO FIFO PARA QUE INTERRUMPA CON 4 BYTES
  Chip_UART_SetupFIFOS( lpcUarts[uartN->perif].uartAddr,
                          UART_FCR_FIFO_EN |
                          UART_FCR_TX_RS   |
                          UART_FCR_RX_RS   |
                          UART_FCR_TRG_LEV0 );
  //uartRxInterruptSet(uartN->perif, TRUE);
  //uartN->modo = MODO_BYTES;
  taskEXIT_CRITICAL();
}

/**
 * @fn void tareaTransmisionUART (void* taskParam )
 *
 * @brief tarea bloqueadea por queTransmision que descarga en fifo de la UART tx
 *        y avisa mediante un callback
 *
 */


void tareaTransmisionUART (void* taskParam ) {
  poolInfo_t*itemQueue;

  uint8_t j;
  int32_t i = 0;


  while(1) {

    xQueueReceive(queTransmision, &itemQueue, portMAX_DELAY);
    i = 0;
    // intento tomar semaforo de thre
    xSemaphoreTake(uartPC.tx_thre, portMAX_DELAY);

    // paso a vaciar el buffer sobre la fifo
    for(j = 0; j < itemQueue->bufL; j++) {

     if((i + 1) >=  FIFO_UART_L){
         xSemaphoreTake(uartPC.tx_thre, portMAX_DELAY);
         i = 0;
     }

     Chip_UART_SendByte(lpcUarts[uartPC.perif].uartAddr, itemQueue->buf[j]);
     i++;
    }

    // cuando termine de mandar, aviso usando el id del token
    xQueueSend(uartPC.queTokenACT, &(itemQueue->token->id_de_paquete), portMAX_DELAY);
    // descargarBufferEnFIFOUARTTx(uartPC.perif, itemQueue->buf, itemQueue->bufL);
  }

}

/**
 * @fn int32_t descargarBufferEnFIFOUARTTx (uartMap_t uartPerif, const char* buf, uint8_t n)
 *
 * @brief descargo datos en fifo de tx en tramos del tamaño de la FIFO
 */


int32_t descargarBufferEnFIFOUARTTx (uartMap_t uartPerif, const uint8_t* buf, uint8_t n) {

  uint8_t j;
  int32_t i = 0;

  // intento tomar semaforo de thre
  xSemaphoreTake(uartPC.tx_thre, portMAX_DELAY);

  // paso a vaciar el buffer sobre la fifo
  for(j = 0; j < n; j++) {

    if((i + 1) >=  FIFO_UART_L){
        xSemaphoreTake(uartPC.tx_thre, portMAX_DELAY);
        i = 0;
    }

    Chip_UART_SendByte(lpcUarts[uartPerif].uartAddr, buf[j]);
    i++;
  }
}


__attribute__ ((section(".after_vectors")))

/**
 * @fn void UART2_IRQHandler(void)
 *
 * @brief handler de la uart PC
 *
 */

void UART2_IRQHandler(void) {

  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  uint8_t byteLeido;

 static uint32_t i;
 uartQueue_t itemQueue;

 char byteRx;

 uartMap_t uart = uartPC.perif;

  // levanto el registro de causa de interrupcion
  uint32_t regIIR = Chip_UART_ReadIntIDReg(lpcUarts[uart].uartAddr);
  uint32_t regFCR;
  uint32_t status = Chip_UART_ReadLineStatus(lpcUarts[uart].uartAddr );
  uint32_t intId;

  // pregunto si tengo una IRQ pendiente
  if(regIIR & UART_IIR_INTSTAT_PEND) {
     return;
  }

  // copio la causa de irq
  intId = regIIR & UART_IIR_INTID_MASK;

  switch(intId) {

   case UART_IIR_INTID_RLS:      /*!< Interrupt identification: Receive line interrupt */
     break;
   case UART_IIR_INTID_RDA:      /*!< Interrupt identification: Receive data available interrupt */
     if(status & UART_LSR_RDR) { // uartRxReady

         byteLeido = Chip_UART_ReadByte( lpcUarts[uart].uartAddr );

         // Chip_UART_SendByte(lpcUarts[uart].uartAddr, byteLeido);
         xQueueSendFromISR(uartPC.queueRxUART, (const void *) &byteLeido, &xHigherPriorityTaskWoken);
         // xQueueSend(uartPC.queueRxUART, (const void *) &byteLeido, portMAX_DELAY);
     }
     else {
         // si llego aca es porque hubo algun error en la trama
     }

     break;
   case UART_IIR_INTID_CTI:      /*!< Interrupt identification: Character time-out indicator interrupt */
     // reinicio fifo de recepcion
     // Chip_UART_SetupFIFOS(lpcUarts[uartPC.perif].uartAddr, UART_FCR_RX_RS);
     break;
   case UART_IIR_INTID_THRE:     /*!< Interrupt identification: THRE interrupt */

     // Tx Interrupt

     if( (status & UART_LSR_THRE) || (status & UART_LSR_TEMT) ) {
         xSemaphoreGiveFromISR(uartPC.tx_thre, &xHigherPriorityTaskWoken);

     }
     else {

     }
  }

  portYIELD_FROM_ISR ( xHigherPriorityTaskWoken );

}
