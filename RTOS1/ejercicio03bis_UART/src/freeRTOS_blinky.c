#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"

#include "antirreb.h"

#include "uart_sw.h"

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

/**
 * @fn void tareaEnviarLetras( void* taskParmPtr )
 *
 * @brief tarea que envia una letra cada vez que apreto un boton
 */

#define TAREA_ENVIAR_LETRAS_TS    100

void tareaEnviarLetras ( void* taskParmPtr ) {

  char letraEnvio = 'a';

  sw_uart_config(200);

  while (TRUE) {

      if(antirreb_tecla1.t == TECLA_PRESIONADA){

          antirreb_tecla1.t = TECLA_SUELTA;

           if(bytePendiente == FALSE) {
              sw_uart_sent(letraEnvio);
              letraEnvio++;
          }
      }

      vTaskDelay(TAREA_ENVIAR_LETRAS_TS / portTICK_RATE_MS);

  }


}



/**
 * @fn funcion ppal
 *
 */

int main(void)
{
   boardConfig();


   xTaskCreate(tareaLed, (const char *) "tareaLed", configMINIMAL_STACK_SIZE, 0,
                     tskIDLE_PRIORITY+1UL, 0);

   xTaskCreate(tareaAntirreboteTEC1, (const char*)"ANTIRREB", configMINIMAL_STACK_SIZE, 0,
                        tskIDLE_PRIORITY+2UL, 0);

   xTaskCreate(tareaEnviarLetras, (const char*)"envioletra", configMINIMAL_STACK_SIZE, 0,
                           tskIDLE_PRIORITY+2UL, 0);

   xTaskCreate(tareaUART, (const char*)"uartgpio", configMINIMAL_STACK_SIZE, 0,
                           tskIDLE_PRIORITY+3UL, 0);





   vTaskStartScheduler();

   while(1);

   return 0;
}
