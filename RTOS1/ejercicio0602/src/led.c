/*
 * led.c
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
 */


/* ---------------------- includes ---------------------------------- */
// Includes de FreeRTOS

#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sapi.h"        // <= Biblioteca sAPI

#include "led.h"
#include "antirreb.h"
#include "gpio.h"

#include "uart.h"

/* ---------------------- proto. de funciones ---------------------------------- */

void tareaMedicionFlancos( void* taskParmPtr );

void tareaParpadearLedG( void* taskParmPtr );
void tareaParpadearLed1( void* taskParmPtr );
void tareaParpadearLed2( void* taskParmPtr );
// void tareaActualizarPeriodoLeds( void* taskParmPtr );

/* ---------------------- variables globales ---------------------------------- */

TickType_t tiempoEncendidoLedG = 200;
TickType_t tiempoApagadoLedG = 200;

TickType_t tiempoEncendidoLed1 = 200;
TickType_t tiempoApagadoLed1 = 200;

estadoMedicionFlancos_t estadoMedicionFlancos = PRIMER_FLANCO;
xSemaphoreHandle semaforoLed2;


/* ---------------------- implementacion de funciones ---------------------------------- */

/**
 * @fn void tareaMedicionFlancos( void* taskParmPtr )
 *
 * @brief tarea que parpadea el led G
 */
void tareaMedicionFlancos( void* taskParmPtr ){

  teclaFlanco_t flancoValido;
  teclaFlanco_t flancoValido1;
  teclaFlanco_t flancoValido2;

  static gpioMap_t teclaPrimerFlanco;
  TickType_t tiempoEntreFlancos;

  uart_t itemColaUART;

  semaforoLed2 = xSemaphoreCreateBinary();

  while (TRUE) {

      xQueueReceive(queueFlancosValidados, &flancoValido, portMAX_DELAY);
      switch (estadoMedicionFlancos)
      {
        case PRIMER_FLANCO:

          flancoValido1 = flancoValido;
          estadoMedicionFlancos = SEGUNDO_FLANCO;
          break;

        case SEGUNDO_FLANCO:
          flancoValido2 = flancoValido;

          // primero analizo el caso de haber tocado una sola tecla
          if(flancoValido1.tecla == flancoValido2.tecla) {
              // xSemaphoreGive(semaforoUART);
              strcpy(itemColaUART.mensaje, "se pulso solo una tecla");
          }
          else {
              // calculo diferencia entre flancos
              tiempoEntreFlancos = flancoValido2.tiempoFlanco - flancoValido1.tiempoFlanco;
              if(flancoValido1.tecla == TEC1)
                sprintf(itemColaUART.mensaje, "{1:2:%d}", tiempoEntreFlancos);
              else if(flancoValido1.tecla == TEC2)
                sprintf(itemColaUART.mensaje, "{2:1:%d}", tiempoEntreFlancos);

              xSemaphoreGive(semaforoLed2);
          }
          xQueueSend(queueUART, &itemColaUART, portMAX_DELAY );
          estadoMedicionFlancos = PRIMER_FLANCO;

          break;



        default:
          estadoMedicionFlancos = PRIMER_FLANCO;

      }


  }


  return;
}



/**
 * @fn void tareaParpadearLedG( void* taskParmPtr )
 *
 * @brief tarea que parpadea el led G cuando se toca la tecla 1
 */
void tareaParpadearLedG( void* taskParmPtr ) {
  TickType_t tiempoInicio;

  while(TRUE) {

      xSemaphoreTake(antirreb_tecla1.semaforoLed, portMAX_DELAY);

      tiempoInicio = xTaskGetTickCount();
      gpioWrite(LEDG, ON);
      vTaskDelayUntil(&tiempoInicio, tiempoEncendidoLedG);
      gpioWrite(LEDG, OFF);
  }
  return;
}


/**
 * @fn void tareaParpadearLedG( void* taskParmPtr )
 *
 * @brief tarea que parpadea el led 1 cuando se toca la tecla 2
 */
void tareaParpadearLed1( void* taskParmPtr ) {
  TickType_t tiempoInicio;

  while(TRUE) {

    xSemaphoreTake(antirreb_tecla2.semaforoLed, portMAX_DELAY);

    tiempoInicio = xTaskGetTickCount();
    gpioWrite(LED1, ON);
    vTaskDelayUntil(&tiempoInicio, tiempoEncendidoLed1);
    gpioWrite(LED1, OFF);
  }
  return;
}


/**
 * @fn void tareaParpadearLed2( void* taskParmPtr )
 *
 * @brief tarea que parpadea el led 2 cuando se hace una medicion
 */
void tareaParpadearLed2( void* taskParmPtr ) {
  TickType_t tiempoInicio;

  while(TRUE) {

    xSemaphoreTake(semaforoLed2, portMAX_DELAY);

    tiempoInicio = xTaskGetTickCount();
    gpioWrite(LED2, ON);
    vTaskDelayUntil(&tiempoInicio, tiempoEncendidoLed1);
    gpioWrite(LED2, OFF);
  }
  return;
}
/**
 * @fn void tareaParpadearLedG( void* taskParmPtr )
 *
 * @brief tarea que parpadea el led G
 */
/*
void tareaActualizarPeriodoLeds( void* taskParmPtr ) {
  static tecQueue_t teclaISR;
  antirreb_t teclaPresionada;

  while(1) {

      // falta validar "tiempoPresionado" antes de aplicarlo

    xQueueReceive(teclasPresionadas, &teclaPresionada, portMAX_DELAY);
    if(teclaPresionada.teclaPin == TEC1) {
      tiempoEncendidoLedG = antirreb_tecla1.tiempoPresionado;
      tiempoApagadoLedG = antirreb_tecla1.tiempoPresionado;
    }
    else if(teclaPresionada.teclaPin == TEC2   ) {
        tiempoEncendidoLed1 = antirreb_tecla2.tiempoPresionado;
        tiempoApagadoLed1 = antirreb_tecla2.tiempoPresionado;
    }
  }


  return;
}
*/

