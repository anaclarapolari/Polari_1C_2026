/*! @mainpage Proyecto 2 Ejercicio 1 - Medidor de distancia por ultrasonido

 * @section genDesc Descripción General
 *
 * Resuelve el ejercicio 1 del proyecto 2: 
 * 
 *      Diseñar el firmware modelando con un diagrama de flujo de manera que cumpla con las 
 *      siguientes funcionalidades:
 *      
 *     * Mostrar distancia medida utilizando los leds de la siguiente manera:
 *          Si la distancia es menor a 10 cm, apagar todos los LEDs.
 *          Si la distancia está entre 10 y 20 cm, encender el LED_1.
 *          Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 *          Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * 
 *     * Mostrar el valor de distancia en cm utilizando el display LCD.
 * 
 *     * Usar TEC1 para activar y detener la medición.
 * 
 *     * Usar TEC2 para mantener el resultado (“HOLD”).
 * 
 *     * Refresco de medición: 1 s
 *
 *      Se deberá conectar a la EDU-ESP un sensor de ultrasonido HC-SR04 y una pantalla LCD y 
 *      utilizando los drivers provistos por la cátedra implementar la aplicación correspondiente. 
 *      Se debe subir al repositorio el código. Se debe incluir en la documentación, realizada 
 *      con doxygen, el diagrama de flujo. 
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
#define DISTANCE_THRESHOLD_1 10
#define DISTANCE_THRESHOLD_2 20
#define DISTANCE_THRESHOLD_3 30
#define MEASUREMENT_PERIOD_MS 1000  // 1 segundo para medir
#define KEY_READ_PERIOD_MS 10       // 10 milisegundos para leer los switches

/*==================[internal data definition]===============================*/

/**
 * @brief Indica si el sistema está realizando mediciones.
 */
static bool medida = false;      // si es true, el sensor mide; si es false, el sistema se apaga

/**
 * @brief Indica si el display LCD debe mantener el último valor mostrado.
 */
static bool hold_mode = false;      // si es true, el display "congela" el número, pero se sigue midiendo

/**
 * @brief Almacena el valor de la última distancia medida en cm.
 */
static uint16_t last_distance = 0;  /*!< Última distancia medida */

/**
 * @brief Almacena el estado previo de los switches para detectar flancos.
 */
static int8_t last_switches = 0;    /*!< Estado anterior de switches */

/**
 * @brief Handle de la tarea encargada de la medición y visualización.
 */
TaskHandle_t measurement_task_handle = NULL; // identificadores de las tareas

/**
 * @brief Handle de la tarea encargada de la lectura de switches.
 */
TaskHandle_t key_read_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Controla el encendido de LEDs según la distancia medida.
 * @param distance Distancia actual en centímetros.
 */
static void ControlLedsBasedOnDistance(uint16_t distance);

/**
 * @brief Tarea de FreeRTOS que gestiona la medición del sensor y actualización de periféricos.
 * @param pvParameter Puntero a parámetros de la tarea (no utilizado).
 */
static void MeasurementTask(void *pvParameter);

/**
 * @brief Tarea de FreeRTOS que gestiona la lectura de los pulsadores TEC1 y TEC2.
 * @param pvParameter Puntero a parámetros de la tarea (no utilizado).
 */
static void KeyReadTask(void *pvParameter);

/*==================[internal functions definition]==========================*/

static void KeyReadTask(void *pvParameter) {
    while (true) {
        int8_t current_switches = SwitchesRead(); // 00000001 si apretas TEC1

        int8_t pressed_switches = current_switches & ~last_switches; // lo hago para resolver el problema de seleccionar por mucho tiempo y que pase más de 10ms
                                                                    // 00000001 & 11111110 = 00000000 -> no detecta nada
        // Procesar TEC1 (SWITCH_1): deja de medir o vuelve a medir
        if (pressed_switches & SWITCH_1) {
            medida = !medida;
        }

        // Procesar TEC2 (SWITCH_2): congela el display o lo descongela
        if (pressed_switches & SWITCH_2) {
            if (medida) {
                hold_mode = !hold_mode;
            }
        }

        last_switches = current_switches;

        vTaskDelay(KEY_READ_PERIOD_MS / portTICK_PERIOD_MS);
    }
}

static void MeasurementTask(void *pvParameter) {
    while (true) {
        if (medida) {
            last_distance = HcSr04ReadDistanceInCentimeters(); // lee la distancia
            ControlLedsBasedOnDistance(last_distance); // compara la distancia y enciende leds

            if (!hold_mode) {
                // Modo normal: mostrar la medida actual en LCD
                if (last_distance <= 999) {
                    LcdItsE0803Write(last_distance);
                }
            }
        } else {
            // Cuando no está midiendo, apagar todo
            LedsOffAll();
            LcdItsE0803Off();
        }

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
/**
 * @brief Función principal de la aplicación. Inicializa periféricos y crea las tareas de FreeRTOS.
 */
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
    // paso la función, el nombre de la tarea, el espacio en memoria, parámetros, prioridad y ID
    xTaskCreate(&KeyReadTask, "KeyReadTask", 1024, NULL, 5, &key_read_task_handle);

    // Crear tarea de medición (cada 1 segundo)
    xTaskCreate(&MeasurementTask, "MeasurementTask", 2048, NULL, 5, &measurement_task_handle);
}

/*==================[end of file]============================================*/