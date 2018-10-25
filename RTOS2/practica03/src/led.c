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
int32_t inicializarManejadorEventoTec1 (void);
void manejadorEventoLedTec1 (Evento_t * evn);

/* ---------------------- variables globales ---------------------------------- */

TickType_t tiempoEncendidoLedG = 200;
TickType_t tiempoApagadoLedG = 200;

TickType_t tiempoEncendidoLed1 = 200;
TickType_t tiempoApagadoLed1 = 200;

estadoMedicionFlancos_t estadoMedicionFlancos = PRIMER_FLANCO;
xSemaphoreHandle semaforoLed2;


/* -------------- */
estadoLedTecX_t estadoLedTec1 = LEDTEC_INACTIVO;



/* ---------------------- implementacion de funciones ---------------------------------- */

/**
 * @file int32_t inicializarManejadorEventoTec1 (void)
 *
 * @brief pongo el manejador de eventos en su estado inicial
 */
int32_t inicializarManejadorEventoTec1 (void) {
  gpioWrite(LEDR, OFF);
  estadoLedTec1 = LEDTEC_APAGADO;
  return 0;

}

/**
 * @file void manejadorEventoLedTec1 (Evento_t * evn)
 *
 * @brief manejo de sgns
 */
void manejadorEventoLedTec1 (Evento_t * evn) {

  switch(estadoLedTec1) {
    case LEDTEC_INACTIVO:

      switch(evn->signal){

      case SIG_MODULO_INICIAR:
        gpioWrite(LEDR, OFF);
        estadoLedTec1 = LEDTEC_APAGADO;
        break;

      default:
        break;

      }
      break;

    case LEDTEC_APAGADO:

      switch(evn->signal){
        case SIG_BOTON_PULSADO:
          gpioWrite(LEDR, ON);
          estadoLedTec1 = LEDTEC_ENCENDIDO;
          break;

        case SIG_BOTON_LIBERADO:
          inicializarManejadorEventoTec1();
          break;

        default:
          break;

      }
      break;

    case LEDTEC_ENCENDIDO:

      switch(evn->signal){
      case SIG_BOTON_PULSADO:
        break;
      case SIG_BOTON_LIBERADO:
        gpioWrite(LEDR, OFF);
        estadoLedTec1 = LEDTEC_APAGADO;
        break;
      default:
        inicializarManejadorEventoTec1();
      }
      break;

    default:
      inicializarManejadorEventoTec1();
      break;


  }

}



