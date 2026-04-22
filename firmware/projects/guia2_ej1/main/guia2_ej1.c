/*! @mainpage Proyecto 2 Ejercicio 1 - Sensor de Distancia HC-SR04 con LEDs y LCD
 *
 * @section genDesc Descripción General
 *
 * Aplicación que controla LEDs según la distancia medida por sensor HC-SR04
 * y muestra la distancia en display LCD.
 *
 * @section hardConn Conexión de Hardware
 *
 * |   HC-SR04      |   EDU-ESP	|
 * |:--------------:|:-------------:|
 * | 	Vcc 	    |	5V      	|
 * | 	Echo		| 	GPIO_3		|
 * | 	Trig	 	| 	GPIO_2		|
 * | 	Gnd 	    | 	GND     	|
 *
 * |   LCD          |   EDU-ESP	|
 * |:--------------:|:-------------:|
 * | 	Vcc 	    |	5V      	|
 * | 	BCD1		| 	GPIO_20		|
 * | 	BCD2	 	| 	GPIO_21		|
 * | 	BCD3	 	| 	GPIO_22		|
 * | 	BCD4	 	| 	GPIO_23		|
 * | 	SEL1	 	| 	GPIO_19		|
 * | 	SEL2	 	| 	GPIO_18		|
 * | 	SEL3	 	| 	GPIO_9		|
 * | 	Gnd 	    | 	GND     	|
 * 
 * @section funcDesc Descripción Funcional
 *
 * - Si distancia < 10 cm: Apagar todos los LEDs
 * - Si 10 cm <= distancia < 20 cm: Encender LED_1 (verde)
 * - Si 20 cm <= distancia < 30 cm: Encender LED_1 y LED_2 (verde y amarillo)
 * - Si distancia >= 30 cm: Encender LED_1, LED_2 y LED_3 (verde, amarillo y rojo)
 * - Mostrar distancia en cm en el display LCD
 *
 * @author Ana Clara Polari (ana.polari@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "gpio_mcu.h"
#include "delay_mcu.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
#define HC_SR04_ECHO_GPIO   GPIO_3
#define HC_SR04_TRIGGER_GPIO GPIO_2

#define DISTANCE_THRESHOLD_1 10  // cm
#define DISTANCE_THRESHOLD_2 20  // cm
#define DISTANCE_THRESHOLD_3 30  // cm

#define MEASUREMENT_PERIOD_MS 1000  // 1 segundo
#define KEY_READ_PERIOD_MS 10       // 10 milisegundos

/*==================[internal data definition]===============================*/
static bool medida = false;      /*!< Flag para indicar si está midiendo */
static bool hold_mode = false;      /*!< Flag para modo HOLD */
static uint16_t last_distance = 0;  /*!< Última distancia medida */
static int8_t last_switches = 0;    /*!< Estado anterior de switches */

TaskHandle_t measurement_task_handle = NULL;
TaskHandle_t key_read_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Controla los LEDs según la distancia medida
 * @param distance Distancia en centímetros
 */
static void ControlLedsBasedOnDistance(uint16_t distance);

/**
 * @brief Tarea para medir distancia periódicamente (cada 1 segundo)
 */
static void MeasurementTask(void *pvParameter);

/**
 * @brief Tarea para leer teclas periódicamente (cada 10 ms)
 */
static void KeyReadTask(void *pvParameter);

/*==================[internal functions definition]==========================*/

/**
 * @brief Tarea para leer teclas periódicamente (cada 10 ms)
 */
static void KeyReadTask(void *pvParameter) {
    while (true) {
        // Leer estado actual de los switches
        int8_t current_switches = SwitchesRead();

        // Detectar presiones (cambio de 0 a 1)
        int8_t pressed_switches = current_switches & ~last_switches; // lo hago para resolver el problema de seleccionar por mucho tiempo y que pase más de 10ms

        // Procesar TEC1 (SWITCH_1)
        if (pressed_switches & SWITCH_1) {
            medida = !medida;
        }

        // Procesar TEC2 (SWITCH_2)
        if (pressed_switches & SWITCH_2) {
            if (medida) {
                hold_mode = !hold_mode;
            }
        }

        // Guardar estado actual para la próxima lectura
        last_switches = current_switches;

        // Esperar 10 milisegundos
        vTaskDelay(KEY_READ_PERIOD_MS / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Tarea para medir distancia periódicamente (cada 1 segundo)
 */
static void MeasurementTask(void *pvParameter) {
    while (true) {
        if (medida) {
            // SIEMPRE leer la distancia (incluso en HOLD)
            last_distance = HcSr04ReadDistanceInCentimeters();

            // SIEMPRE controlar los LEDs según la medida actual
            ControlLedsBasedOnDistance(last_distance);

            // Actualizar LCD según el modo
            if (!hold_mode) {
                // Modo normal: mostrar la medida actual en LCD
                if (last_distance <= 999) {
                    LcdItsE0803Write(last_distance);
                }
                printf("Distancia: %u cm\n", last_distance);
            }
        } else {
            // Cuando no está midiendo, apagar todo
            LedsOffAll();
            LcdItsE0803Off();
        }

        // Esperar 1 segundo
        vTaskDelay(MEASUREMENT_PERIOD_MS / portTICK_PERIOD_MS);
    }
}

static void ControlLedsBasedOnDistance(uint16_t distance) {
    if (distance < DISTANCE_THRESHOLD_1) {
        // Distancia < 10 cm: Apagar todos los LEDs
        LedsOffAll();
    } else if (distance < DISTANCE_THRESHOLD_2) {
        // Distancia 10-20 cm: Encender solo LED_1
        LedsOffAll();
        LedOn(LED_1);
    } else if (distance < DISTANCE_THRESHOLD_3) {
        // Distancia 20-30 cm: Encender LED_1 y LED_2
        LedsOffAll();
        LedOn(LED_1);
        LedOn(LED_2);
    } else {
        // Distancia >= 30 cm: Encender LED_1, LED_2 y LED_3
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }
}

/*==================[external functions definition]==========================*/
void app_main(void) {
   
    // Inicializar LEDs
    if (!LedsInit()) {
        return;
    }

    // Inicializar sensor HC-SR04
    if (!HcSr04Init(HC_SR04_ECHO_GPIO, HC_SR04_TRIGGER_GPIO)) {
        LedsOffAll();
        return;
    }

    // Inicializar display LCD
    if (!LcdItsE0803Init()) {
        LedsOffAll();
        HcSr04Deinit();
        return;
    }
    // Inicializar switches
    if (SwitchesInit() == -1) {
        LedsOffAll();
        HcSr04Deinit();
        LcdItsE0803DeInit();
        return;
    }

    // Crear tarea de lectura de teclas (cada 10 ms)
    xTaskCreate(&KeyReadTask, "KeyReadTask", 1024, NULL, 5, &key_read_task_handle);

    // Crear tarea de medición (cada 1 segundo)
    xTaskCreate(&MeasurementTask, "MeasurementTask", 2048, NULL, 5, &measurement_task_handle);
}

/*==================[end of file]============================================*/