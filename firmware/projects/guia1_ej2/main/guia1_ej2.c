/*! @mainpage Proyecto 1 Ejercicio 2 - Blinking switch
 *
 * \section genDesc General Description
 * 
 * Resuelve el ejercicio 2 del proyecto 1: 
 * 
 *      Modifique la aplicación 1_blinking_switch de manera de hacer titilar los leds 1 y 2 
 *      al mantener presionada las teclas 1 y 2 correspondientemente. También se debe poder 
 *      hacer titilar el led 3 al presionar simultáneamente las teclas 1 y 2. 
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

    LedsInit(); // configura los GPIO como OUTPUT (si no lo hago, el pin no envía corriente y no se va a prender el LED nunca)
    SwitchesInit(); // configura los GPIO de los botones como INPUT

    while(1) {
        teclas = SwitchesRead();      /* máscara de teclas pulsadas */
 
        switch(teclas){
            case SWITCH_1:           /* sólo pulsada la tecla 1 */
                LedToggle(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
                break;

            case SWITCH_2:           /* sólo pulsada la tecla2 */
                LedToggle(LED_2);
                LedOff(LED_1);
                LedOff(LED_3);
                break;

            case (SWITCH_1|SWITCH_2):   /* ambas teclas pulsadas */
                LedToggle(LED_3);
                LedOff(LED_1);
                LedOff(LED_2);
                break;

            default:                 /* ninguna tecla pulsada */
                LedOff(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
                break;
        }

        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}
/*==================[end of file]============================================*/