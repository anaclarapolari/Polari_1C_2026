/*! @mainpage Proyecto 2 Ejercicio 3 - Medidor de distancia por ultrasonido c/interrupciones y puerto serie

 * @section genDesc Descripción General

 * Resuelve el ejercicio 3 del proyecto 2: 
 * 
 *      Cree un nuevo proyecto en el que modifique la actividad del punto 2 agregando ahora el 
 * 		puerto serie. Envíe los datos de las mediciones para poder observarlos en un terminal 
 * 		en la PC, siguiendo el siguiente formato:
 * 
 * 			* 3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + 
 * 				cambio de línea “ \r\n”
 * 			* Además debe ser posible controlar la EDU-ESP de la siguiente manera: Con las 
 * 				teclas “O” y “H”, replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP
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
#include "uart_mcu.h"

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
 * @brief Procesa los comandos recibidos por UART.
 * * @param param Puntero opcional a parámetros (no utilizado).
 */
static void ProcesarUART(void *param);

/** * @brief Configuración del puerto serie (UART).
 */
static serial_config_t uart_pc_config = {
    .port = UART_PC,
    .baud_rate = 9600,
    .func_p = ProcesarUART,
    .param_p = NULL
};

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
/**
 * @brief Maneja la interrupción de TEC1. Altera el estado de la medición (encendido/apagado).
 * * @param param Puntero a parámetro no utilizado.
 */
static void InterrupcionTEC1(void *param) {
    medida = !medida;
    
    // Si se desactiva la medición, apagar todo
    if (!medida) {
        LedsOffAll();
        LcdItsE0803Off();
    }
}
/**
 * @brief Maneja la interrupción de TEC2. Activa o desactiva el modo de retención (HOLD).
 * * @param param Puntero a parámetro no utilizado.
 */
static void InterrupcionTEC2(void *param) {
    if (medida) {
        hold_mode = !hold_mode;
    }
}
/**
 * @brief Mide la distancia y actualiza la pantalla, los LEDs y envía los datos por UART.
 * * @param param Puntero a parámetro no utilizado.
 */
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
		// Envío de datos por UART
        char tx_buffer[16]; //guarda caracteres, 16 es el tamaño: 3 digitos, 1 espacio, 2 letras, 2 caracteres invisibles y bit de stop (elijo 16 porque 8 es muy poco)
        sprintf(tx_buffer, "%03d cm\r\n", last_distance); 
        UartSendString(UART_PC, tx_buffer);
    }
}
/**
 * @brief Cambia el estado de los LEDs según el umbral de la distancia.
 * * @param distance Distancia calculada por el HC-SR04.
 */
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
/**
 * @brief Procesa el dato recibido por UART y llama a la función correspondiente según el comando.
 * * @param param Puntero a parámetro no utilizado.
 */
static void ProcesarUART(void *param) {
    // Aquí se procesa la recepción de datos por UART
    uint8_t rx_data;
    UartReadByte(UART_PC, &rx_data);

    // Replicar la funcionalidad de TEC1 con 'O' y TEC2 con 'H'
    if (rx_data == 'O' || rx_data == 'o') {
        InterrupcionTEC1(NULL);
    } else if (rx_data == 'H' || rx_data == 'h') {
        InterrupcionTEC2(NULL);
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

    // configuro UART
    UartInit(&uart_pc_config);
}
/*==================[end of file]============================================*/