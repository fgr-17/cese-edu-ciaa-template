/*
 * led.c
 *
 *  Created on: 17 ago. 2018
 *      Author: froux
 */


/* ---------------------- includes ---------------------------------- */
#include "sapi.h"        // <= Biblioteca sAPI
// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "gpio.h"

#include "antirreb.h"

/* ---------------------- proto. de funciones ---------------------------------- */

void tareaParpadearLedG( void* taskParmPtr );
void tareaParpadearLed1( void* taskParmPtr );
void tareaActualizarPeriodoLeds( void* taskParmPtr );

/* ---------------------- variables globales ---------------------------------- */

TickType_t tiempoEncendidoLedG = 200;
TickType_t tiempoApagadoLedG = 200;

TickType_t tiempoEncendidoLed1 = 200;
TickType_t tiempoApagadoLed1 = 200;

/* ---------------------- implementacion de funciones ---------------------------------- */

/**
 * @fn void tareaParpadearLedG( void* taskParmPtr )
 *
 * @brief tarea que parpadea el led G
 */
void tareaParpadearLedG( void* taskParmPtr ) {
  TickType_t tiempoInicio;

  while(TRUE) {
    tiempoInicio = xTaskGetTickCount();
    gpioWrite(LEDG, ON);
    vTaskDelayUntil(&tiempoInicio, tiempoEncendidoLedG);
    gpioWrite(LEDG, OFF);
    vTaskDelayUntil(&tiempoInicio, tiempoEncendidoLedG + tiempoApagadoLedG);
  }
  return;
}


/**
 * @fn void tareaParpadearLedG( void* taskParmPtr )
 *
 * @brief tarea que parpadea el led G
 */
void tareaParpadearLed1( void* taskParmPtr ) {
  TickType_t tiempoInicio;

  while(TRUE) {
    tiempoInicio = xTaskGetTickCount();
    gpioWrite(LED1, ON);
    vTaskDelayUntil(&tiempoInicio, tiempoEncendidoLed1);
    gpioWrite(LED1, OFF);
    vTaskDelayUntil(&tiempoInicio, tiempoEncendidoLed1 + tiempoApagadoLed1);
  }
  return;
}
/**
 * @fn void tareaParpadearLedG( void* taskParmPtr )
 *
 * @brief tarea que parpadea el led G
 */

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


