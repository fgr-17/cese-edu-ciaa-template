/*
 * uart.c
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
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
#include "hm10.h"
#include "uart.h"

/* ---------------------------- prototipos --------------------------------- */
int32_t inicializarStructUart (uart_t*uart, uartMap_t perif, uint32_t baudrate);
int32_t inicializarTareaEnviarDatosUARTs ( void );
void tareaEnviarDatosUART ( void* taskParmPtr );
void tareaRecibirStringPorTimeout (void* uartN);

/* ---------------------------- variables globales --------------------------------- */

uart_t uartPC;
uart_t uartBLE;

/* ---------------------------- implementacion --------------------------------- */


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
 * @fn void tareaRecibirStringPorTimeout (void* uartN)
 *
 * @brief recibo una cadena por timeout
 *
 */

void tareaRecibirStringPorTimeout (void* uartN) {

  uart_t* uartCast;
  uartQueue_t itemQueueRx;
  uartCast = uartN;
  int32_t i;

  char mensajeRecibido [MENSAJE_L];

  static UBaseType_t mensajesEsperando;

  while(TRUE) {

      // espero que se haya iniciado una IRQ
      xSemaphoreTake(uartCast->semaphoreInicioIRQ, portMAX_DELAY);

      // corto la recepcion si la cantidad de bytes es mayor, espero a una nueva llamada al callbackComun

      mensajesEsperando = uxQueueMessagesWaiting(uartCast->queueRxStringUART);

      if(mensajesEsperando) {

        i = 0;

        while(TRUE) {

            // recibo por desde el callback comun que manda por la cola RxStringUART y lo tiro en mensajeRecibido
            if(xQueueReceive(uartCast->queueRxStringUART, &mensajeRecibido[i],UART_DELAY_RX_ENTRE_CARACTERES) == pdFALSE) {
                // entra a esta condición cuando xQueueReceive salio por timeout
                // corto el final del string
                mensajeRecibido[i] = '\0';
                strcpy(itemQueueRx.mensaje, mensajeRecibido);
                xQueueSend(uartCast->queueRxUART, &itemQueueRx, portMAX_DELAY);
                // uartCast->semaphoreInicioIRQ = xSemaphoreCreateCounting(MENSAJE_L, 0);
                break;
            }


            i++;

            if((i + 1) > MENSAJE_L) {
                mensajeRecibido[i] = '\0';
                strcpy(itemQueueRx.mensaje, mensajeRecibido);
                xQueueSend(uartCast->queueRxUART, &itemQueueRx, portMAX_DELAY);
                i = 0;
            }


        }
      }

    }

}

/**
 * @fn int32_t inicializarTareaEnviarDatosUART ( void ) {
 *
 * @brief inicializo colas de datos de las teclas
 *
 */

int32_t inicializarStructUart (uart_t*uart, uartMap_t perif, uint32_t baudrate) {

  uart->perif = perif;
  uart->baudrate = baudrate;
  uart->queueTxUART = xQueueCreate (QUEUE_UART_L, sizeof(uartQueue_t));
  uart->queueRxUART = xQueueCreate (QUEUE_UART_L, sizeof(uartQueue_t));
  uart->queueRxStringUART = xQueueCreate (FIFO_UART_L*2, sizeof(char));
  // uart->semaphoreInicioIRQ = xSemaphoreCreateCounting(FIFO_UART_L*2, 0);
  uart->modo = MODO_BYTES;
  uart->semaphoreInicioIRQ = xSemaphoreCreateBinary();
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
 /* Chip_UART_SetupFIFOS( lpcUarts[uartPC.perif].uartAddr,
                        UART_FCR_FIFO_EN |
                        UART_FCR_TX_RS   |
                        UART_FCR_RX_RS   |
                        UART_FCR_TRG_LEV1 );
*/
  uartRxInterruptSet(UART_PC, TRUE);


  inicializarStructUart(&uartBLE, UART_BLE, UART_BLE_BAUDRATE);
  uartInit( uartBLE.perif, uartBLE.baudrate);
  uartRxInterruptSet(UART_BLE, TRUE);


  return 0;
}

/**
 * @fn int32_t descargarStringEnFIFOUARTTx (uartMap_t uartPerif, const char* string)
 *
 * @brief descargo datos en fifo de tx en tramos del tamaño de la FIFO
 */

int32_t descargarStringEnFIFOUARTTx (uartMap_t uartPerif, const char* string) {

  int32_t i;



  if(Chip_UART_ReadLineStatus( lpcUarts[uartPerif].uartAddr ) & UART_LSR_THRE) {

      while (TRUE) {

          Chip_UART_SendByte(lpcUarts[uartPerif].uartAddr, *string );
          string++;
          i++;

          if(*string == '\0')
            return 0;

          if((i + 1) >=  FIFO_UART_L) {
              while(!(Chip_UART_ReadLineStatus( lpcUarts[uartPerif].uartAddr ) & UART_LSR_THRE));
              i = 0;
          }

        }

  }
  else
    return -1;


}

/**
 * @fn void tareaEnviarDatosUART ( void* taskParmPtr )
 *
 * @brief envio lo que esta en la cola Tx
 *
 */

void tareaEnviarDatosUART ( void* uartN ) {

  uartQueue_t itemColaUART;
  uart_t* uartCast;
  char cadena [25] = "\n\nInicializacion UART\n\n";

  uartCast = uartN;

  // queueUART = xQueueCreate (QUEUE_UART_L, sizeof(uart_t));
  // uartWriteString (uartCast->perif, cadena);

  while (TRUE) {

      xQueueReceive(uartCast->queueTxUART, &itemColaUART, portMAX_DELAY);
      // uartWriteString(uartCast->perif, itemColaUART.mensaje);
      descargarStringEnFIFOUARTTx(uartCast->perif, itemColaUART.mensaje);
      // hacerlo con enviar byte? Chip_UART_SendByte(lpcUarts

  }

}

/**
 * @fn void uartCallbackUmbral(uartMap_t uart)
 *
 * @brief handler de la uart PC
 *
 */

void uartCallbackUmbral(uart_t*uartStr, BaseType_t*xHigherPriorityTaskWoken)
{
   uartQueue_t itemQueue;
   uartMap_t uart;
   uart = uartStr->perif;

   // levanto el registro de causa de interrupcion
   uint32_t regIIR = Chip_UART_ReadIntIDReg(lpcUarts[uart].uartAddr);
   uint32_t regFCR;
   uint32_t status = Chip_UART_ReadLineStatus( lpcUarts[uart].uartAddr );
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

         itemQueue.mensaje[0]= Chip_UART_ReadByte( lpcUarts[uart].uartAddr );
         itemQueue.mensaje[1]= Chip_UART_ReadByte( lpcUarts[uart].uartAddr );
         itemQueue.mensaje[2]= Chip_UART_ReadByte( lpcUarts[uart].uartAddr );
         itemQueue.mensaje[3]= Chip_UART_ReadByte( lpcUarts[uart].uartAddr );
         xQueueSendFromISR(uartStr->queueRxUART, (const void *) &itemQueue, xHigherPriorityTaskWoken);
         return;
       }
       else {
           // si llego aca es porque hubo algun error en la trama
           return;
       }

       break;
     case UART_IIR_INTID_CTI:      /*!< Interrupt identification: Character time-out indicator interrupt */
       // reinicio fifo de recepcion

       itemQueue.mensaje[0]= Chip_UART_ReadByte( lpcUarts[uart].uartAddr );
       itemQueue.mensaje[1]= Chip_UART_ReadByte( lpcUarts[uart].uartAddr );
       itemQueue.mensaje[2]= Chip_UART_ReadByte( lpcUarts[uart].uartAddr );

       regFCR = lpcUarts[uart].uartAddr->FCR;
       lpcUarts[uart].uartAddr->FCR |= regFCR | UART_FCR_RX_RS;
       return;

       // Chip_UART_SetupFIFOS(lpcUarts[uartPC.perif].uartAddr, UART_FCR_RX_RS);
       break;
     case UART_IIR_INTID_THRE:     /*!< Interrupt identification: THRE interrupt */

       // Tx Interrupt

       if( (status & UART_LSR_THRE) || (status & UART_LSR_TEMT) ) {


       }
       else {

       }
       return;
   }


   return;
}



/**
 * @fn void uartCallbackComun(uartMap_t uart)
 *
 * @brief handler de la uart PC
 *
 */

void uartCallbackComun(uart_t*uartStr, BaseType_t*xHigherPriorityTaskWoken)
{

   uint8_t byteLeido;
   static uint32_t i;
   uartQueue_t itemQueue;

   char byteRx;

   uartMap_t uart = uartStr->perif;

   // levanto el registro de causa de interrupcion
   uint32_t regIIR = Chip_UART_ReadIntIDReg(lpcUarts[uart].uartAddr);
   uint32_t regFCR;
   uint32_t status = Chip_UART_ReadLineStatus( lpcUarts[uart].uartAddr );
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

           // if(uxSemaphoreGetCount(uartStr->semaphoreInicioIRQ))
           if(xSemaphoreGiveFromISR(uartStr->semaphoreInicioIRQ, xHigherPriorityTaskWoken) != pdTRUE)
             i = 1;

           i = 0;

           do {

               byteRx = Chip_UART_ReadByte( lpcUarts[uart].uartAddr );

               if(byteRx != '\0')
                 xQueueSendFromISR(uartStr->queueRxStringUART, (const void *) &byteRx, xHigherPriorityTaskWoken);


               i++;

           }while(byteRx!= '\0');

           return;
       }
       else {
           // si llego aca es porque hubo algun error en la trama
           return;
       }

       break;
     case UART_IIR_INTID_CTI:      /*!< Interrupt identification: Character time-out indicator interrupt */
       // reinicio fifo de recepcion
       return;

       // Chip_UART_SetupFIFOS(lpcUarts[uartPC.perif].uartAddr, UART_FCR_RX_RS);
       break;
     case UART_IIR_INTID_THRE:     /*!< Interrupt identification: THRE interrupt */

       // Tx Interrupt

       if( (status & UART_LSR_THRE) || (status & UART_LSR_TEMT) ) {


       }
       else {

       }
       return;

   }


   return;
}
__attribute__ ((section(".after_vectors")))
/**
 * @fn void UART3_IRQHandler(void)
 *
 * @brief handler de la uart BLE
 *
 */

void UART3_IRQHandler(void) {

  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if(uartBLE.modo == MODO_BYTES)
    uartCallbackComun(&uartBLE, &xHigherPriorityTaskWoken);

  else if(uartBLE.modo == MODO_UMBRAL)
    uartCallbackUmbral(&uartBLE, &xHigherPriorityTaskWoken);

  portYIELD_FROM_ISR ( xHigherPriorityTaskWoken );
}

/**
 * @fn void UART2_IRQHandler(void)
 *
 * @brief handler de la uart PC
 *
 */
void UART2_IRQHandler(void) {

  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;


  if(uartPC.modo == MODO_BYTES)
    uartCallbackComun(&uartPC, &xHigherPriorityTaskWoken);

  else if(uartPC.modo == MODO_UMBRAL)
    uartCallbackUmbral(&uartPC, &xHigherPriorityTaskWoken);

  portYIELD_FROM_ISR ( xHigherPriorityTaskWoken );

}
