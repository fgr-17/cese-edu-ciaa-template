/**
 * @file manejoEventos.c
 *
 * @brief funciones de aplicacion del framework eventos
 *
 * @author Federico Roux
 *
 */

#include <stdint.h>
#include "FrameworkEventos.h"

#include "sapi.h"


#include "manejoEventos.h"
#include "antirreb.h"
#include "led.h"
/* --------------------- prototipos -------------------------- */

int32_t  inicializarColasEventos(void);
int32_t inicializarModulos (void);
void manejadorEventoPruebaTout(Evento_t * evn);
/* --------------------- variables globales -------------------------- */

Modulo_t * ModuloBroadcast;
Modulo_t * moduloTec;
Modulo_t * ModuloLedTec1;
Modulo_t * ModuloLedTec2;
Modulo_t * ModuloLedTec3;
Modulo_t * ModuloLedTec4;
Modulo_t * moduloLedPrueba;


/* --------------------- funciones de inicializacion -------------------------- */

/**
 * @fn int32 inicializarColasEventos(void)
 *
 * @brief inicializo las colas de datos para eventos
 */

int32_t inicializarColasEventos(void) {

  queEventosBaja = xQueueCreate(COLA_EVENTOS_BAJA_PRIORIDAD_LARGO, sizeof(Evento_t));
  return 0;
}

/**
 * @fn int32_t inicializarModulos (void)
 *
 * @brief registro todos los modulos
 */
int32_t inicializarModulos (void) {

  ModuloBroadcast = RegistrarModulo(ManejadorEventosBroadcast, PRIORIDAD_BAJA);
  moduloTec = RegistrarModulo(manejadorEventosPulsador, PRIORIDAD_BAJA);
  ModuloLedTec1 = RegistrarModulo(manejadorEventoLedTec1, PRIORIDAD_BAJA);
  // moduloLedPrueba = RegistrarModulo(manejadorEventoPruebaTout, PRIORIDAD_BAJA);

/*
  ModuloLedTec2 = RegistrarModulo(ManejadorEventosBroadcast, PRIORIDAD_BAJA);
  ModuloLedTec3 = RegistrarModulo(ManejadorEventosBroadcast, PRIORIDAD_BAJA);
  ModuloLedTec4 = RegistrarModulo(ManejadorEventosBroadcast, PRIORIDAD_BAJA);
*/



  IniciarTodosLosModulos();
  return 0;
}

void manejadorEventoPruebaTout(Evento_t * evn) {
   switch(evn->signal){
     case SIG_MODULO_INICIAR:
       timerArmarRepetitivo(moduloLedPrueba, 1000);

       break;
     case SIG_TIMEOUT:
        gpioToggle(LED1);
        break;
   }
  return;
}
