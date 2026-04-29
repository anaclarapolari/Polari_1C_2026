/*! @mainpage Proyecto 2 Ejercicio 2 - Medidor de distancia por ultrasonido con interrupciones

 * @section genDesc
 *
 * @section hardConn 
 *
 * @section funcDesc 
 * 
 * @section intDesc
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

#define HC_SR04_ECHO_GPIO   GPIO_3
#define HC_SR04_TRIGGER_GPIO GPIO_2

#define DISTANCE_THRESHOLD_1 10
#define DISTANCE_THRESHOLD_2 20
#define DISTANCE_THRESHOLD_3 30

#define MEASUREMENT_PERIOD_US 1000000  // 1 segundo

/*==================[internal data definition]===============================*/
static bool medida = false;      /*!< Flag para indicar si está midiendo */
static bool hold_mode = false;   /*!< Flag para modo HOLD */
static uint16_t last_distance = 0;  /*!< Última distancia medida */

/*==================[internal functions declaration]=========================*/
static void ControlLedsBasedOnDistance(uint16_t distance);

static void MeasurementTimerCallback(void *param);
static void Switch1Callback(void *param);
static void Switch2Callback(void *param);

/*==================[internal functions definition]==========================*/
static void Switch1Callback(void *param) {
    medida = !medida;
    
    // Si se desactiva la medición, apagar todo
    if (!medida) {
        LedsOffAll();
        LcdItsE0803Off();
    }
}

static void Switch2Callback(void *param) {
    if (medida) {
        hold_mode = !hold_mode;
    }
}

static void MeasurementTimerCallback(void *param) {
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
        } else {
            // Cuando no está midiendo, apagar todo
            LedsOffAll();
            LcdItsE0803Off();
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

    // Configurar interrupciones de switches con callbacks
    SwitchActivInt(SWITCH_1, Switch1Callback, NULL);
    SwitchActivInt(SWITCH_2, Switch2Callback, NULL);

    // Configurar timer para medición periódica (cada 1 segundo = 1,000,000 us)
    timer_config_t timer_config = {
        .timer = TIMER_A,
        .period = MEASUREMENT_PERIOD_US,
        .func_p = MeasurementTimerCallback,
        .param_p = NULL
    };
    TimerInit(&timer_config);
    TimerStart(TIMER_A);

/*==================[end of file]============================================*/