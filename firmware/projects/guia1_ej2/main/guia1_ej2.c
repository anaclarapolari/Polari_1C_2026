/*! @mainpage Blinking switch
 *
 * \section genDesc General Description
 *
 * Este ejemplo hace titilar los leds en función de las teclas.
 *  * LED1 titila mientras se mantiene pulsada SWITCH1.
 *  * LED2 titila mientras se mantiene pulsada SWITCH2.
 *  * LED3 titila cuando se mantienen simultáneamente ambas teclas.
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
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    uint8_t teclas;

    LedsInit();
    SwitchesInit();

    while(1) {
        teclas = SwitchesRead();      /* máscara de teclas pulsadas */

        switch(teclas){
            case SWITCH_1:           /* sólo pulsada la 1 */
                LedToggle(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
                break;

            case SWITCH_2:           /* sólo pulsada la 2 */
                LedToggle(LED_2);
                LedOff(LED_1);
                LedOff(LED_3);
                break;

            case (SWITCH_1|SWITCH_2):/* ambas pulsadas */
                LedToggle(LED_3);
                LedOff(LED_1);
                LedOff(LED_2);
                break;

            default:                 /* ninguna pulsada */
                LedOff(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
                break;
        }

        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}