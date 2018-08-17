/*
 * antirreb.h
 *
 *  Created on: Apr 6, 2018
 *      Author: piro2
 */

/* ========================== [ declaracion de constantes ] ======================= */

/** @brief delay de la ventana de antirrebote */
#define ANTIRREBOTE_DELAY	40
/** @brief valor logico de tecla presionada */
#define TECLA_ABAJO			OFF
/** @brief valor logico de tecla suelta */
#define TECLA_ARRIBA		ON
/** @brief periodo de muestreo de la rutina antirrebote */
#define ANTIRREB_TS     50
/** @brief largo de la cola de datos de teclas presionadas */
#define TECLAS_PRESIONADAS_L      4

/* ========================== [ definicion de tipos de datos ] ======================= */

/** @brief enumeracion de estados de la maquina de estados de antirrebote */
typedef enum {BUTTON_UP, BUTTON_FALLING, BUTTON_DOWN, BUTTON_RAISING} antirreboteMEF_t;
/** @brief enumeracion de valores del flag de tecla */
typedef enum {TECLA_PRESIONADA, TECLA_SUELTA, TECLA_ERROR} tecla_t;

typedef struct
{

  antirreboteMEF_t estado;
	delay_t delay;

	gpioMap_t teclaPin;
	tecla_t t;
	TickType_t tiempoPresionIni;
	TickType_t tiempoPresionFin;
	TickType_t tiempoPresionado;
	TickType_t tiempoVentana;

}antirreb_t;


/* ========================== [ funciones externas ] ======================= */

extern int inicializarQueueTeclasPresionadas ( void );

extern void tareaAntirreboteTEC1 ( void* taskParmPtr );
extern void tareaAntirreboteTEC2 ( void* taskParmPtr );

extern void inicializarAntirreboteMEF (antirreb_t*antirreb, gpioMap_t tecla_asigada );
extern void antirreboteMEF (antirreb_t*antirreb);

/* ========================== [ variables externas ] ======================= */
extern antirreb_t antirreb_tecla1;
extern antirreb_t antirreb_tecla2;
extern antirreb_t antirreb_tecla3;
extern antirreb_t antirreb_tecla4;

extern QueueHandle_t teclasPresionadas;

