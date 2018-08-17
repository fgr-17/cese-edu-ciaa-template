/* Author: Gaston Vallasciani
 * Materia: Sistemas operativos de tiempo real 1
 * Fecha: 12/08/2018
 *
 * Ejercicio 7: Colas e ISR
 *
 */

/*==================[inclusiones]============================================*/
// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"   // Hay que agregarlo siempre para poder utilizar un semaforo
#include "isrPinLevel.h"

// sAPI header
#include "sapi.h"

/*==================[definiciones y macros]==================================*/
#define DEBOUNCE_TIME 20
#define TURN_OFF 0
#define PRESSED 0
#define FREE 0
#define RELEASED 1
#define	FLANCO_DESCENDENTE 2
#define	FLANCO_ASCENDENTE 3

typedef enum {BUTTON_UP, BUTTON_FALLING, BUTTON_DOWN, BUTTON_RISING} fsmDebounce_t;

typedef struct{
	gpioMap_t tecla;
	fsmDebounce_t stateActual;
	delay_t delay;
}debounceData_t;


/*==================[definiciones de datos internos]=========================*/
TickType_t tiempo_global;
SemaphoreHandle_t semaforo;
QueueHandle_t cola;
uint8_t LED = LEDR;
/*==================[definiciones de datos externos]=========================*/

DEBUG_PRINT_ENABLE;

/*==================[declaraciones de funciones internas]====================*/
void DEBOUNCE_fsmDebounceInit(debounceData_t * dataStruct, gpioMap_t tec);
uint32_t DEBOUNCE_fsmDebounceAct(debounceData_t * ptrDataStruct);
/*==================[declaraciones de funciones externas]====================*/

// Prototipo de funcion de la tarea
void showLED1( void* taskParmPtr );
void debounce( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{
   // ---------- CONFIGURACIONES ------------------------------
   // Inicializar y configurar la plataforma
   boardConfig();

   // UART for debug messages
   debugPrintConfigUart( UART_USB, 115200 );
   debugPrintlnString( "Blinky con freeRTOS y sAPI." );

   // Led para dar senal de vida
   gpioWrite( LED3, ON );

   semaforo = xSemaphoreCreateBinary();
   cola = xQueueCreate( 5, sizeof(colaStruct_t));

   // Creacion de la tarea showTime
   xTaskCreate(
	  showLED1,                     // Funcion de la tarea a ejecutar
      (const char *)"showLED1",     // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, 	  // Cantidad de stack de la tarea
      0,                              // Parametros de tarea
      tskIDLE_PRIORITY+2,             // Prioridad de la tarea
      0                               // Puntero a la tarea creada en el sistema
   );

   // Creacion tarea debounce
   xTaskCreate(
   	  debounce,                     // Funcion de la tarea a ejecutar
      (const char *)"debounce",     // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, 	  // Cantidad de stack de la tarea
      0,                              // Parametros de tarea
      tskIDLE_PRIORITY+1,             // Prioridad de la tarea
      0                               // Puntero a la tarea creada en el sistema
   );

   isrPinLevelConfig();
   // Iniciar scheduler
   vTaskStartScheduler();

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE ) {
      // Si cae en este while 1 significa que no pudo iniciar el scheduler
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void showLED1( void* taskParmPtr )
{
	TickType_t tiempo_inicio_ciclo;

	// ---------- CONFIGURACIONES ------------------------------
	gpioWrite( LED, OFF );

   // ---------- REPETIR POR SIEMPRE --------------------------
   while(TRUE) {
	   if(xSemaphoreTake( semaforo , 1000) == pdTRUE){
		   gpioWrite( LED, ON );
		   vTaskDelay( tiempo_global );
		   gpioWrite( LED, OFF );
      }
	   else{

	   }
   }
}

void debounce( void* taskParmPtr )
{
	TickType_t tiempo_inicio_ciclo, tiempo_fin_de_ciclo, tiempo_medido;
//	uint32_t flanco;
	colaStruct_t elemento_quitado;

	// ---------- CONFIGURACIONES ------------------------------

   // ---------- REPETIR POR SIEMPRE --------------------------
   while(TRUE) {

	   xQueueReceive( cola, &elemento_quitado, portMAX_DELAY );

	   switch(elemento_quitado.flanco){
	   case FREE:
		   break;
	   case FLANCO_DESCENDENTE:
		   tiempo_inicio_ciclo = elemento_quitado.countTick;
		   debugPrintlnString( "FLANCO DESCENDENTE" );
		   break;
	   case FLANCO_ASCENDENTE:
		   debugPrintlnString( "FLANCO ASCENDENTE" );
		   tiempo_fin_de_ciclo = elemento_quitado.countTick;
		   tiempo_medido = tiempo_fin_de_ciclo - tiempo_inicio_ciclo;
		   if (tiempo_medido>20*portTICK_RATE_MS){
			   tiempo_global = tiempo_medido;
			   tiempo_inicio_ciclo = 0;
			   tiempo_fin_de_ciclo = 0;
			   tiempo_medido = 0;
			   switch(elemento_quitado.tecIndex){
			   case 0x01:
				   LED = LEDG;
				   break;
			   case 0x02:
				   LED = LED1;
				   break;
			   case 0x03:
				   LED = LED2;
				   break;
			   case 0x04:
				   LED = LED3;
				   break;
			   }
			   xSemaphoreGive( semaforo );
		   }
		   tiempo_inicio_ciclo = 0;
		   tiempo_fin_de_ciclo = 0;
		   tiempo_medido = 0;
		   break;
	   }
   }
}


/*Funcion que inicializa la maquina de estados que controla la mef de debounce.
 *Esta funcion debe ser llamada en el main.c antes del superloop*/
void DEBOUNCE_fsmDebounceInit(debounceData_t * dataStruct, gpioMap_t tec){
	dataStruct -> tecla = tec;
	dataStruct -> stateActual = BUTTON_UP;

	return;
}

/*Funcion que actualiza la mef y entrega el flanco encontrado*/
uint32_t DEBOUNCE_fsmDebounceAct(debounceData_t * ptrDataStruct){
	uint32_t flanco = TURN_OFF;
	bool_t tecState = OFF;

	//Lee el estado de la tecla
	tecState = gpioRead(ptrDataStruct->tecla);

	switch(ptrDataStruct -> stateActual){

	case BUTTON_UP:
		if(tecState == PRESSED){
			ptrDataStruct->stateActual = BUTTON_FALLING;
		}
		break;
	case BUTTON_FALLING:
		vTaskDelay( 20 / portTICK_RATE_MS );
			if(tecState == PRESSED){
				ptrDataStruct->stateActual = BUTTON_DOWN;
				// detecta el flanco descendente y lo entrega a la salida
				flanco = FLANCO_DESCENDENTE;
			}else{
				ptrDataStruct->stateActual = BUTTON_UP;
			}
		break;
	case BUTTON_DOWN:

		if (tecState == RELEASED){
			ptrDataStruct->stateActual = BUTTON_RISING;
		}
		break;
	case BUTTON_RISING:
		vTaskDelay( 20 / portTICK_RATE_MS );
			if(tecState == RELEASED){
				//  Espera 20ms y se vuelve a fijar Si la tecla fue liberada. Si esto ocurrio pasa al estado button up
				ptrDataStruct->stateActual = BUTTON_UP;
				// detecta el flanco ascendente y lo entrega a la salida
				flanco = FLANCO_ASCENDENTE;
			}else{
				ptrDataStruct->stateActual = BUTTON_DOWN;
			}
		break;
	//Estado de error: resetea la mef
	default:
		ptrDataStruct -> stateActual = BUTTON_UP;
		break;

	}
	return(flanco);
}


/*==================[fin del archivo]========================================*/
