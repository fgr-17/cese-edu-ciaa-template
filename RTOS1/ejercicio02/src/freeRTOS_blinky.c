#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

DEBUG_PRINT_ENABLE;

StackType_t myTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t myTaskTCB;


#define TON_MIN         100
#define TON_MAX         900

#define TON_INICIAL     0
#define TON_INCREMENTO  100

#define PERIODO         1000



void myTask( void* taskParmPtr )
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

        vTaskDelayUntil(&tiempoInicio, PERIODO / portTICK_RATE_MS);
   }
}

int main(void)
{
   boardConfig();

   xTaskCreateStatic(myTask, "myTask", configMINIMAL_STACK_SIZE, NULL,
                     tskIDLE_PRIORITY+1, myTaskStack, &myTaskTCB);

   vTaskStartScheduler();

   while(1);

   return 0;
}
