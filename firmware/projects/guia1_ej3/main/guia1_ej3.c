/*! @mainpage Proyecto 1 Ejercicio 3 - Blinking switch con uso de estructuras
 *
 * @section genDesc General Description
 *
 * Resuelve el ejercicio 3 del proyecto 1: 
 * 
 *  	El programa permite controlar el estado de los LEDs (encendido, apagado o parpadeo) 
 * 		utilizando una estructura de datos.
 * 
 *      Realice un función que reciba un puntero a una estructura LED como la que se muestra.
 * 		Use como guía para la implementación el diagrama de flujo que se observa en la 
 * 		carpeta doc.
 *
 * @author Ana Clara Polari (ana.polari@ingenieria.uner.edu.ar)
 *
 */
/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

struct leds
{
      uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;        //indica el número de led a controlar
	uint8_t n_ciclos;     //indica la cantidad de ciclos de encendido/apagado
	uint16_t periodo;     //indica el tiempo de cada ciclo
} my_leds;  //my_leds es el objeto de tipo struct leds que ocupa espacio en memoria

enum {ON, OFF, TOGGLE}; //ON=0, OFF=1, TOGGLE=2

/*==================[internal functions declaration]=========================*/

void parpadeo_led (struct leds*leds)  // la función recibe un PUNTERO a una estructura (la direc)
{
	switch (leds->mode)
	{
		case ON:
			LedOn(leds->n_led);
			break;
		case OFF:
			LedOff(leds->n_led);
			break;
		case TOGGLE:
			for(int i=0; i<2*leds->n_ciclos; i++) //un ciclo de parpadeo completo requiere dos cambios de estado (prender y apagar)
			{
				LedToggle(leds->n_led);
				vTaskDelay(leds->periodo / portTICK_PERIOD_MS);
			}
			break;
	}
}

/*==================[external functions definition]==========================*/

void app_main(void){
    LedsInit();

    my_leds.mode = TOGGLE;
    my_leds.n_led = LED_1;
    my_leds.n_ciclos = 10;
    my_leds.periodo = 100;

    parpadeo_led(&my_leds); //& significa "dirección de"
}

/*==================[end of file]============================================*/