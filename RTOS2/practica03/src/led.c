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

#include "FrameworkEventos.h"

#include "led.h"
#include "antirreb.h"
#include "gpio.h"

#include "uart.h"

/* ---------------------- manejadores de eventos ---------------------------------- */
int32_t inicializarManejadorEventoTec (void);
void manejadorEventoLed (Evento_t * evn);

/* ---------------------- variables globales ---------------------------------- */

TickType_t tiempoEncendidoLedG = 200;
TickType_t tiempoApagadoLedG = 200;

TickType_t tiempoEncendidoLed1 = 200;
TickType_t tiempoApagadoLed1 = 200;

estadoMedicionFlancos_t estadoMedicionFlancos = PRIMER_FLANCO;
xSemaphoreHandle semaforoLed2;


/* -------------- */
estadoLedTecX_t estadoLedTec1 = LEDTEC_INACTIVO;

// el led azul no se debe prender nunca
gpioMap_t ledsTeclas[] = {LEDB, LEDR, LED1, LED2, LED3};


/* ---------------------- implementacion de funciones ---------------------------------- */

/**
 * @file int32_t inicializarManejadorEventoTec (void)
 *
 * @brief pongo el manejador de eventos en su estado inicial
 */
int32_t inicializarManejadorEventoTec (void) {
  gpioWrite(ledsTeclas[0], OFF);
  gpioWrite(ledsTeclas[1], OFF);
  gpioWrite(ledsTeclas[2], OFF);
  gpioWrite(ledsTeclas[3], OFF);
  gpioWrite(ledsTeclas[4], OFF);
  return 0;

}

/**
 * @file void manejadorEventoLed (Evento_t * evn)
 *
 * @brief manejo de sgns
 */
void manejadorEventoLed (Evento_t * evn) {

  int teclaPulsada;

  switch(evn->signal){

  case SIG_MODULO_INICIAR:
    inicializarManejadorEventoTec();
    break;

  case SIG_BOTON_PULSADO:
    teclaPulsada = evn->valor;
    gpioWrite(ledsTeclas[teclaPulsada], ON);
    break;

  case SIG_BOTON_LIBERADO:
    teclaPulsada = evn->valor;
    gpioWrite(ledsTeclas[teclaPulsada], OFF);
    break;

  default:
    inicializarManejadorEventoTec();
  }

}



