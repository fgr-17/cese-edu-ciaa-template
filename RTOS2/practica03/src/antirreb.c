/*
 * antirreb.c
 *
 *  Created on: Apr 6, 2018
 *      Author: piro2
 */



#include <stdint.h>


#include "FreeRTOS.h"

#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "sapi.h"        // <= Biblioteca sAPI


#include "FrameworkEventos.h"
#include "manejoEventos.h"


#include "gpio.h"
#include "antirreb.h"

#include "uart.h"

/* ======================= [variables globales] ======================= */

antirreb_t antirreb_tecla1;
antirreb_t antirreb_tecla2;
antirreb_t antirreb_tecla3;
antirreb_t antirreb_tecla4;

QueueHandle_t teclasPresionadas;
QueueHandle_t queueFlancosValidados;

/* ======================= [implementacion de funciones] ======================= */

int inicializarQueuesFlancosyTeclas ( void ) {

  // en lugar de hacer la cola con la struct completa, solo me interesa saber que tecla y cuanto tiempo estuvo presionada
  if( (teclasPresionadas = xQueueCreate (TECLAS_PRESIONADAS_L, sizeof(antirreb_t))) == NULL)   return 1;
  if( (queueFlancosValidados = xQueueCreate (QUEUE_FLANCOS_VALIDADOS_L, sizeof(teclaFlanco_t))) == NULL)   return 1;

  return 0;
}


/**
 * @fn void Inicializar_fsmAntirrebote (void)
 *
 * @brief inicializacion de la maquina de estados.
 */

void inicializarAntirreboteMEF (antirreb_t*antirreb, gpioMap_t tecla_asigada, int teclaN ){

  antirreb->teclaN = teclaN;
	antirreb->estado = BUTTON_UP;
	antirreb->tiempoVentana = ANTIRREBOTE_DELAY;
	antirreb->teclaPin = tecla_asigada;
	return;
}

/**
 *
 * @fn void fsmAntirrebote (void)
 *
 * @brief maquina de estados antirrebote parametrizada
 *
 */

void antirreboteMEF (antirreb_t*antirreb){

  teclaFlanco_t flancoValido;
  static tecQueue_t teclaISR;

  char cadenaTxt [100];

	bool_t tecValue = TECLA_ARRIBA;

	switch(antirreb->estado)
	{

	case BUTTON_UP:
		// tecValue = gpioRead( antirreb->teclaPin );
	  // evaluo si el dato para sacar corresponde a esta instancia de la MEF
	  if (gpioRead(antirreb->teclaPin) == TECLA_PRESIONADA) {

      antirreb->estado = BUTTON_FALLING;
	  }
		break;

	case BUTTON_FALLING:

		/* Si el tiempo del delay expiró, paso a leer el estado del pin */
	  tecValue = gpioRead( antirreb->teclaPin );
    if(tecValue == TECLA_ARRIBA){
      antirreb->estado = BUTTON_UP;
    }
    else if(tecValue == TECLA_ABAJO){
      antirreb->estado = BUTTON_DOWN;
      EncolarEvento(moduloInformePulsacion, SIG_BOTON_PULSADO, antirreb->teclaN);
      EncolarEvento(moduloLedTec, SIG_BOTON_PULSADO, antirreb->teclaN);
    }
		break;
	case  BUTTON_DOWN:

    // evaluo si el dato para sacar corresponde a esta instancia de la MEF
	  if (gpioRead(antirreb->teclaPin) == TECLA_SUELTA) {
	      antirreb->estado = BUTTON_RAISING;
    }
		break;

	case BUTTON_RAISING:

		/* Si el tiempo del delay expiró, paso a leer el estado del pin */
			tecValue = gpioRead( antirreb->teclaPin );
			if(tecValue == TECLA_ABAJO){
				antirreb->estado = BUTTON_DOWN;
			}
			else if(tecValue == TECLA_ARRIBA){

        antirreb->estado = BUTTON_UP;
        EncolarEvento(moduloInformePulsacion, SIG_BOTON_LIBERADO, antirreb->teclaN);
	      EncolarEvento(moduloLedTec, SIG_BOTON_LIBERADO, antirreb->teclaN);
			}
		break;

	default:
		inicializarAntirreboteMEF(antirreb, antirreb->teclaPin, antirreb->teclaN);

	}

	return;
}



/**
 * @fn void manejadorEventosPulsador (Evento_t * evn)
 *
 * @brief fsm que maneja los pulsadores
 */



void manejadorEventosPulsador (Evento_t * evn){

  int tecla_i;

  switch(evn->signal)
  {
    case SIG_MODULO_INICIAR:
      timerArmarRepetitivo(moduloTec, 50);
      inicializarAntirreboteMEF(&antirreb_tecla1, TEC1, 1);
      inicializarAntirreboteMEF(&antirreb_tecla2, TEC2, 2);
      inicializarAntirreboteMEF(&antirreb_tecla3, TEC3, 3);
      inicializarAntirreboteMEF(&antirreb_tecla4, TEC4, 4);
      break;
    case SIG_TIMEOUT:
      antirreboteMEF(&antirreb_tecla1);
      antirreboteMEF(&antirreb_tecla2);
      antirreboteMEF(&antirreb_tecla3);
      antirreboteMEF(&antirreb_tecla4);
      break;
    default:
      break;

  }

  return;
}


