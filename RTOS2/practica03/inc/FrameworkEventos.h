/*
 * FrameworkEventos.h
 *
 *  Created on: 15/06/2012
 *      Author: alejandro
 */

#ifndef FRAMEWORKEVENTOS_H_
#define FRAMEWORKEVENTOS_H_

#include "FreeRTOS.h"
#include "queue.h"

#include "fe_modulos.h"
#include "fe_eventos.h"
#include "fe_broadcast.h"
#include "fe_seniales.h"
#include "fe_TimerService.h"

enum {
	PRIORIDAD_BAJA = 1,
	PRIORIDAD_MEDIA,
	PRIORIDAD_ALTA,
};

extern Modulo_t modulos[];
extern int ultimoModulo;

#endif /* FRAMEWORKEVENTOS_H_ */
