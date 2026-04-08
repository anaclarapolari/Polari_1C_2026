/**
 * @file guia2_ej1.c
 * @brief Medidor de distancia con HC-SR04, visualización en display 3 dígitos
 *        y señalización de rangos con LEDs.
 *
 * @section flowchart Diagrama de flujo
 *
 *   +-----------------------------+
 *   | INICIO                     |
 *   +-----------------------------+
 *                |
 *   +-----------------------------+
 *   | Inicializar drivers        |
 *   |  - HC-SR04                 |
 *   |  - LCD ITS E0803           |
 *   |  - LEDs                    |
 *   +-----------------------------+
 *                |
 *   +-----------------------------+
 *   | Leer distancia en cm       |
 *   +-----------------------------+
 *                |
 *   +-----------------------------+
 *   | distancia < 10             |
 *   |   Apagar todos los LEDs    |
 *   +-----------------------------+
 *                | no
 *   +-----------------------------+
 *   | distancia < 20             |
 *   |   Encender LED_1           |
 *   +-----------------------------+
 *                | no
 *   +-----------------------------+
 *   | distancia < 30             |
 *   |   Encender LED_2 y LED_1   |
 *   +-----------------------------+
 *                | no
 *   +-----------------------------+
 *   | Encender LED_3, LED_2, LED_1|
 *   +-----------------------------+
 *
 * @section hardware Hardware
 *  HC-SR04 Echo -> GPIO_3
 *  HC-SR04 Trig -> GPIO_2
 *  HC-SR04 Vcc  -> 5V
 *  HC-SR04 GND  -> GND
 *
 *  LCD ITS E0803 BCD1 -> GPIO_20
 *  LCD ITS E0803 BCD2 -> GPIO_21
 *  LCD ITS E0803 BCD3 -> GPIO_22
 *  LCD ITS E0803 BCD4 -> GPIO_23
 *  LCD ITS E0803 SEL1 -> GPIO_19
 *  LCD ITS E0803 SEL2 -> GPIO_18
 *  LCD ITS E0803 SEL3 -> GPIO_9
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "gpio_mcu.h"
#include "delay_mcu.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "led.h"

/*==================[macros and definitions]=================================*/
#define HC_SR04_ECHO_PIN    GPIO_3
#define HC_SR04_TRIGGER_PIN GPIO_2
#define UPDATE_PERIOD_MS    250u
#define LCD_MAX_VALUE       999u

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
static void UpdateDistanceLeds(uint16_t distance_cm);
static void DisplayDistanceOnLCD(uint16_t distance_cm);

/*==================[internal functions definition]==========================*/
static void UpdateDistanceLeds(uint16_t distance_cm){
    if (distance_cm < 10u){
        LedsOffAll();
    }
    else if (distance_cm < 20u){
        LedsMask(LED_1);
    }
    else if (distance_cm < 30u){
        LedsMask(LED_1 | LED_2);
    }
    else{
        LedsMask(LED_1 | LED_2 | LED_3);
    }
}

static void DisplayDistanceOnLCD(uint16_t distance_cm){
    if (distance_cm > LCD_MAX_VALUE){
        distance_cm = LCD_MAX_VALUE;
    }
    LcdItsE0803Write(distance_cm);
}

/*==================[external functions definition]==========================*/
void app_main(void){
    uint16_t distance_cm = 0u;

    printf("Iniciando guia2_ej1 sin ILI9341...\n");

    LedsInit();
    LedsOffAll();

    if (!LcdItsE0803Init()){
        printf("Error: no se pudo inicializar el display 3 dígitos\n");
    }

    if (!HcSr04Init(HC_SR04_ECHO_PIN, HC_SR04_TRIGGER_PIN)){
        printf("Error: no se pudo inicializar HC-SR04\n");
    }

    while (1){
        distance_cm = HcSr04ReadDistanceInCentimeters();

        printf("Distancia: %u cm\n", distance_cm);
        DisplayDistanceOnLCD(distance_cm);
        UpdateDistanceLeds(distance_cm);

        DelayMs(UPDATE_PERIOD_MS);
    }
}
/*==================[end of file]============================================*/