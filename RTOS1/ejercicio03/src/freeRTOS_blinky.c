#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

#include "antirreb.h"

DEBUG_PRINT_ENABLE;

/* -------------------- defines ------------------------------ */

#define TON_MIN         100
#define TON_MAX         900

#define TON_INICIAL     0
#define TON_INCREMENTO  100

#define PERIODO         1000

/* -------------------- funciones ------------------------------ */

void myTask( void* taskParmPtr );

/* -------------------- funciones ------------------------------ */

/**
 * @fn void myTask( void* taskParmPtr )
 *
 * @brief tarea que parpadea el led con tOn incremental
 */
void tareaLed( void* taskParmPtr )
{
	TickType_t tiempoInicio;
	TickType_t tiempoBloqueo;

	TickType_t tOn, tOff;

    gpioWrite( LED2, OFF );

    gpioInit( GPIO1, GPIO_OUTPUT );

    debugPrintConfigUart( UART_USB, 115200 );
    debugPrintlnString( "Blinky con freeRTOS y sAPI." );

    tOn = TON_INICIAL;

    while(TRUE) {


        if(antirreb_tecla1.t == TECLA_PRESIONADA){
            tiempoInicio = xTaskGetTickCount();
            antirreb_tecla1.t = TECLA_SUELTA;
            gpioWrite(LEDG, ON);
            vTaskDelayUntil(&tiempoInicio, antirreb_tecla1.tiempoPresionado / portTICK_RATE_MS);
            gpioWrite(LEDG, OFF);
        }
        vTaskDelay(10 / portTICK_RATE_MS);
/*
        tiempoInicio = xTaskGetTickCount();

        gpioWrite ( LEDR, ON );
        gpioWrite ( GPIO1, ON );

        tOn += TON_INCREMENTO;
        if(tOn > TON_MAX) {
            tOn = TON_MIN;
        }

        tOff = PERIODO - tOn;

        tiempoBloqueo = tOn;

        vTaskDelay(tiempoBloqueo / portTICK_RATE_MS);

        gpioWrite ( LEDR, OFF );
        gpioWrite ( GPIO1, OFF );
*/
   }
}





/**
 * @fn funcion ppal
 *
 */

int main(void)
{
   boardConfig();


   xTaskCreate(tareaLed, (const char *) "myTask", configMINIMAL_STACK_SIZE, 0,
                     tskIDLE_PRIORITY+1UL, 0);

   xTaskCreate(tareaAntirreboteTEC1, (const char*)"ANTIRREB", configMINIMAL_STACK_SIZE, 0,
                        tskIDLE_PRIORITY+2UL, 0);


   vTaskStartScheduler();

   while(1);

   return 0;
}
