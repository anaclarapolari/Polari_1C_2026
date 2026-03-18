/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
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
} my_leds; 

enum {ON, OFF, TOGGLE};

/*==================[internal functions declaration]=========================*/

void parpadeo_led (struct leds*leds)
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
			for(int i=0; i<2*leds->n_ciclos; i++)
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
    my_leds.periodo = 500;

    parpadeo_led(&my_leds);
}

/*==================[end of file]============================================*/