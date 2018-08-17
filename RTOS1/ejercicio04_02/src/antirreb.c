/*
 * antirreb.c
 *
 *  Created on: Apr 6, 2018
 *      Author: piro2
 */

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"        // <= Biblioteca sAPI

#include "antirreb.h"

/* ======================= [variables globales] ======================= */

antirreb_t antirreb_tecla1;
antirreb_t antirreb_tecla2;
antirreb_t antirreb_tecla3;
antirreb_t antirreb_tecla4;


/* ======================= [implementacion de funciones] ======================= */


/**
 * @fn void tareaAntirreboteTeclas (void)
 *
 * @brief tarea que atiende la rutina antirrebote de todas las teclas
 */

void tareaAntirreboteTEC1 ( void* taskParmPtr ) {

  inicializarAntirreboteMEF(&antirreb_tecla1, TEC1);


  while ( TRUE ) {

      antirreboteMEF(&antirreb_tecla1);
      vTaskDelay(ANTIRREB_TS / portTICK_RATE_MS);

  }


  return;
}



/**
 * @fn void Inicializar_fsmAntirrebote (void)
 *
 * @brief inicializacion de la maquina de estados.
 */

void inicializarAntirreboteMEF (antirreb_t*antirreb, gpioMap_t tecla_asigada ){

	antirreb->estado = BUTTON_UP;
	antirreb->t = TECLA_SUELTA;
	antirreb->tiempoVentana = ANTIRREBOTE_DELAY;
	antirreb->teclaPin = tecla_asigada;
}

/**
 *
 * @fn void fsmAntirrebote (void)
 *
 * @brief maquina de estados antirrebote parametrizada
 *
 */

void antirreboteMEF (antirreb_t*antirreb){

	bool_t tecValue = TECLA_ARRIBA;

	switch(antirreb->estado)
	{

	case BUTTON_UP:
		tecValue = gpioRead( antirreb->teclaPin );
		if(tecValue == TECLA_ARRIBA){
			antirreb->estado = BUTTON_UP;
		}
		else if (tecValue == TECLA_ABAJO)
		{
			antirreb->estado = BUTTON_FALLING;

			antirreb->tiempoPresionIni = xTaskGetTickCount();
			vTaskDelay(antirreb->tiempoVentana / portTICK_RATE_MS);
// 			delayConfig(&antirreb->delay, antirreb->tiempo_ventana);
			/* para que el delay arranque hay que leerlo*/
// 			delayRead(&antirreb->delay);
		}

		break;
	case BUTTON_FALLING:

		/* Si el tiempo del delay expiró, paso a leer el estado del pin */
		// if (delayRead(&antirreb->delay)){
    tecValue = gpioRead( antirreb->teclaPin );
    if(tecValue == TECLA_ARRIBA){
      antirreb->estado = BUTTON_UP;
    }
    else if(tecValue == TECLA_ABAJO){
      antirreb->estado = BUTTON_DOWN;
//      antirreb->t = TECLA_PRESIONADA;
    }

//		}
//		else{
//			antirreb->estado = BUTTON_FALLING;
//		}
		break;
	case  BUTTON_DOWN:

		tecValue = gpioRead( antirreb->teclaPin );
		if(tecValue == TECLA_ABAJO){
			antirreb->estado = BUTTON_DOWN;
		}
		else if (tecValue == TECLA_ARRIBA)
		{
			antirreb->estado = BUTTON_RAISING;
			antirreb->tiempoPresionFin = xTaskGetTickCount();
			vTaskDelay(antirreb->tiempoVentana / portTICK_RATE_MS);
			// delayConfig(&antirreb->delay, antirreb->tiempo_ventana);
			/* para que el delay arranque hay que leerlo*/
			// delayRead(&antirreb->delay);
		}
		break;

	case BUTTON_RAISING:


		/* Si el tiempo del delay expiró, paso a leer el estado del pin */
//		if (delayRead(&antirreb->delay)){
			tecValue = gpioRead( antirreb->teclaPin );
			if(tecValue == TECLA_ABAJO){
				antirreb->estado = BUTTON_DOWN;
			}
			else if(tecValue == TECLA_ARRIBA){
				antirreb->estado = BUTTON_UP;
	      antirreb->t = TECLA_PRESIONADA;
	      antirreb->tiempoPresionado = antirreb->tiempoPresionFin - antirreb->tiempoPresionIni;
			}

//		}
//		else{
//			antirreb->estado = BUTTON_RAISING;
//		}
		break;

	default:
		inicializarAntirreboteMEF(antirreb, antirreb->teclaPin);

	}

	return;
}



