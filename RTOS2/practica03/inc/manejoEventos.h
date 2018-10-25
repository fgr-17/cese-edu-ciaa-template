/*
 * manejoEventos.h
 *
 *  Created on: 24 oct. 2018
 *      Author: froux
 */

#ifndef RTOS2_PRACTICA03_INC_MANEJOEVENTOS_H_
#define RTOS2_PRACTICA03_INC_MANEJOEVENTOS_H_


/* --------------------- defines -------------------------- */

#define COLA_EVENTOS_BAJA_PRIORIDAD_LARGO         4




/* --------------------- fc externas -------------------------- */

extern int32_t inicializarColasEventos(void);
extern int32_t inicializarModulos (void);


/* --------------------- vars externas -------------------------- */

extern Modulo_t * ModuloBroadcast;
extern Modulo_t * ModuloLedTec1;
extern Modulo_t * moduloTec;

#endif /* RTOS2_PRACTICA03_INC_MANEJOEVENTOS_H_ */
