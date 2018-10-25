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

#include "qmpool.h"

#include "antirreb.h"
#include "led.h"
#include "protocolo.h"
/* --------------------- prototipos -------------------------- */

int32_t  inicializarColasEventos(void);
int32_t inicializarModulos (void);
void manejadorEventoPruebaTout(Evento_t * evn);
/* --------------------- variables globales -------------------------- */

Modulo_t * ModuloBroadcast;
Modulo_t * moduloTec;
Modulo_t * moduloLedTec;
Modulo_t * moduloLedPrueba;
Modulo_t * moduloInformePulsacion;


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
  moduloLedTec = RegistrarModulo(manejadorEventoLed, PRIORIDAD_BAJA);
  moduloInformePulsacion = RegistrarModulo( manejadorEventoUART, PRIORIDAD_BAJA);
  IniciarTodosLosModulos();
  return 0;
}
/**
 * @fn void manejadorEventoPruebaTout(Evento_t * evn)
 *
 * @brief prueba blinky con sist. reactivo
 */

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
