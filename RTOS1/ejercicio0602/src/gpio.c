/*
 * gpio.c
 *
 *  Created on: 30 jul. 2018
 *      Author: froux
 */

/* ---------------------------- includes --------------------------------- */
/*
*/
// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sapi.h"        // <= Biblioteca sAPI

#include "gpio.h"

DEBUG_PRINT_ENABLE;

/* ---------------------------- prototipos --------------------------------- */
int inicializarQueuesTeclas ( void );
int inicializarTecla (void);

/* ---------------------------- variables globales --------------------------------- */

QueueHandle_t teclasQueue;

/* ---------------------------- funciones --------------------------------- */

/**
 * @fn int inicializarQueuesTeclas ( void )
 *
 * @brief inicializo colas de datos de las teclas
 *
 */
int inicializarQueuesTeclas ( void ) {

  if( (teclasQueue = xQueueCreate (TECLAS_QUEUE_L, sizeof(tecQueue_t))) == NULL)   return 1;


  return 0;
}


/**
 * @fn int inicializarTecla (void)
 *
 * @brief inicializo dos interrupciones para un pin en flanco asc. y desc. x separado
 *
 */

int inicializarTecla (void) {

	/* CONFIGURO ISR (2 HANDLERS PARA EL MISMO PIN) */

  /*Seteo la interrupción para el flanco descendente: (pagina 300 UM10503)
   *                channel, GPIOx, [y]    <- no es la config del pin, sino el nombre interno de la señal
   *                      |  |      |
   *                      |  |      |    */

  Chip_SCU_GPIOIntPinSel( TECLA1_CANAL_DESC, TECLA1_PORT, TECLA1_PIN);
  //Borra el pending de la IRQ
  Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( TECLA1_CANAL_DESC ) ); // INT0 (canal 0 -> hanlder GPIO0)
  //Selecciona activo por flanco
  Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( TECLA1_CANAL_DESC ) ); // INT0
  //Selecciona activo por flanco descendente
  Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( TECLA1_CANAL_DESC ) ); // INT0
  //Selecciona activo por flanco ascendente
  Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( TECLA1_CANAL_ASC ) ); // INT1


  Chip_SCU_GPIOIntPinSel( TECLA2_CANAL_DESC, TECLA2_PORT, TECLA2_PIN);
  //Borra el pending de la IRQ
  Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( TECLA2_CANAL_DESC ) ); // INT0 (canal 0 -> hanlder GPIO0)
  //Selecciona activo por flanco
  Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( TECLA2_CANAL_DESC ) ); // INT0
  //Selecciona activo por flanco descendente
  Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( TECLA2_CANAL_DESC ) ); // INT0
  //Selecciona activo por flanco ascendente
  Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( TECLA2_CANAL_ASC ) ); // INT1

  //Borra el clear pending de la IRQ y lo activa
  NVIC_SetPriority(PIN_INT0_IRQn, 0x7);
  NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
  NVIC_EnableIRQ( PIN_INT0_IRQn );

  NVIC_SetPriority(PIN_INT1_IRQn, 0x7);
  NVIC_ClearPendingIRQ( PIN_INT1_IRQn );
  NVIC_EnableIRQ( PIN_INT1_IRQn );



  return 0;
}


/**
 * @fn void GPIO0_IRQHandler(void)
 *
 * @brief handler de canal 0 de interrupcion de GPIO
 *
 * @author fgr-17
 */

void GPIO0_IRQHandler(void) {

  static uint32_t PININT;
  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  tecQueue_t itemQueue;

  itemQueue.teclaPresionada = TEC1;
  itemQueue.tickEvento = xTaskGetTickCountFromISR();

  if ( Chip_PININT_GetFallStates(LPC_GPIO_PIN_INT) & PININTCH(TECLA1_CANAL_DESC) ) {

      itemQueue.flancoDetectado = FLANCO_BAJADA;
      Chip_PININT_ClearFallStates(LPC_GPIO_PIN_INT,PININTCH(TECLA1_CANAL_DESC));
   }
  else {
      itemQueue.flancoDetectado = FLANCO_SUBIDA;
      Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT,PININTCH(TECLA1_CANAL_ASC));
  }

  xQueueSendFromISR(teclasQueue, (const void *) &itemQueue, &xHigherPriorityTaskWoken);


  portYIELD_FROM_ISR ( xHigherPriorityTaskWoken );
  // TEC1 GPIO0[4]d
  // TEC2 GPIO0[8]
  // TEC3 GPIO0[9]
  return;
}

/**
 * @fn void GPIO1_IRQHandler(void)
 *
 * @brief handler de canal 1 de interrupcion de GPIO
 *
 * @author fgr-17 (rouxfederico@gmail.com)
 */

void GPIO1_IRQHandler(void) {

  static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  tecQueue_t itemQueue;

  itemQueue.teclaPresionada = TEC2;
  itemQueue.tickEvento = xTaskGetTickCountFromISR();

  if ( Chip_PININT_GetFallStates(LPC_GPIO_PIN_INT) & PININTCH(TECLA2_CANAL_DESC) ) {
      itemQueue.flancoDetectado = FLANCO_BAJADA;
      Chip_PININT_ClearFallStates(LPC_GPIO_PIN_INT,PININTCH(TECLA2_CANAL_DESC));
   }
  else {
      itemQueue.flancoDetectado = FLANCO_SUBIDA;
      Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT,PININTCH(TECLA2_CANAL_ASC));
  }
  xQueueSendFromISR(teclasQueue, (const void *) &itemQueue, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR ( xHigherPriorityTaskWoken );
  // TEC4 GPIO1[9]
  return;
}
