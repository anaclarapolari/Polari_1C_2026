/*! @mainpage Proyecto 2 Ejercicio 2 - Medidor de distancia por ultrasonido con interrupciones

 * @section genDesc Descripción General

 * Resuelve el ejercicio 2 del proyecto 2: 
 * 
 *      Cree un nuevo proyecto en el que modifique la actividad del punto 1 de manera de 
 *      utilizar interrupciones para el control de las teclas y el control de tiempos (Timers). 
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
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/

#define HC_SR04_ECHO_GPIO    GPIO_3     /*!< GPIO asignado al pin de ECHO del sensor */
#define HC_SR04_TRIGGER_GPIO GPIO_2     /*!< GPIO asignado al pin de TRIGGER del sensor */
#define DISTANCE_THRESHOLD_1 10         /*!< Umbral de distancia menor (10 cm) */
#define DISTANCE_THRESHOLD_2 20         /*!< Umbral de distancia medio (20 cm) */
#define DISTANCE_THRESHOLD_3 30         /*!< Umbral de distancia mayor (30 cm) */
#define MEASUREMENT_PERIOD_US 1000000  // 1 segundo en microsegundos

/*==================[internal data definition]===============================*/

static bool medida = false;      /*!< Para indicar si está midiendo */
static bool hold_mode = false;   /*!< Para modo HOLD */
static uint16_t last_distance = 0;  /*!< Última distancia medida */

/*==================[internal functions declaration]=========================*/

/**
 * @brief Controla el encendido de LEDs en función de la distancia medida.
 * @param distance Distancia medida en centímetros.
 */
static void ControlLedsBasedOnDistance(uint16_t distance);

/**
 * @brief Función de callback invocada por el timer para realizar la medición.
 * @param param Puntero opcional a parámetros (no utilizado).
 */
static void MedirDistancia(void *param);

/**
 * @brief Función de interrupción asociada a la tecla 1 (TEC1).
 * @param param Puntero opcional a parámetros (no utilizado).
 */
static void InterrupcionTEC1(void *param);

/**
 * @brief Función de interrupción asociada a la tecla 2 (TEC2).
 * @param param Puntero opcional a parámetros (no utilizado).
 */
static void InterrupcionTEC2(void *param);

/*==================[internal functions definition]==========================*/

static void InterrupcionTEC1(void *param) {
    medida = !medida;
    
    // Si se desactiva la medición, apagar todo
    if (!medida) {
        LedsOffAll();
        LcdItsE0803Off();
    }
}

static void InterrupcionTEC2(void *param) {
    if (medida) {
        hold_mode = !hold_mode;
    }
}

static void MedirDistancia(void *param) {
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
        }
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
        LedsOffAll();
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }
}

/*==================[external functions definition]==========================*/
/**
 * @brief Función principal que inicializa periféricos, configura interrupciones y timers.
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

    // configuro interrupciones: paso la tecla, la función a ejecutar y argumentos no hay
    // reemplazo la tarea que leía las teclas cada 10 ms
    SwitchActivInt(SWITCH_1, InterrupcionTEC1, NULL);
    SwitchActivInt(SWITCH_2, InterrupcionTEC2, NULL);

    // configuro timer cada 1 segundo. Cada un segundo se mide la distancia
    timer_config_t timer_config = {
        .timer = TIMER_A,
        .period = MEASUREMENT_PERIOD_US,
        .func_p = MedirDistancia,
        .param_p = NULL
    };
    TimerInit(&timer_config); //le paso la dirección de memoria del timer que configuré recién
	TimerStart(TIMER_A);
}
/*==================[end of file]============================================*/