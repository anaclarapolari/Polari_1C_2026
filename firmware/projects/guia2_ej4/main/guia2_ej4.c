/*! @mainpage Proyecto 2 Ejercicio 4 - Osciloscopio.
 *
 * @section genDesc Descripción General
 *
 * Resuelve el ejercicio 4 del proyecto 2: 
 *
 *      Diseñar e implementar una aplicación, basada en el driver analog io mcu.y el driver de 
 *      transmisión serie uart mcu.h, que digitalice una señal analógica y la transmita a un 
 *      graficador de puerto serie de la PC. Se debe tomar la entrada CH1 del conversor AD y la 
 *      transmisión se debe realizar por la UART conectada al puerto serie de la PC, en un formato 
 *      compatible con un graficador por puerto serie. Debe verificar:
 *
 *          * Disparar la conversión AD a través de una interrupción periódica de timer.
 *          * Utilice una frecuencia de muestreo de 500Hz.
 *          * Obtener los datos en una variable que le permita almacenar todos los bits del conversor.
 *          * Transmitir los datos por la UART en formato ASCII a una velocidad de transmisión 
 *          suficiente para realizar conversiones a la frecuencia requerida.
 *
 *      Prueba del osciloscopio: Convierta una señal digital de un ECG (provista por la cátedra) 
 *      en una señal analógica y visualice esta señal utilizando el osciloscopio que acaba de 
 *      implementar. Se sugiere utilizar el potenciómetro para conectar la salida del DAC a la 
 *      entrada CH1 del AD.
 *      La señal analógica se encuentra en el archivo doc/otro ecg.txt de este proyecto.
 *
 * @section hardConn 
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
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/
#define frec_muestreo_Hz    500U
#define SAMPLE_PERIOD_US    (1000000UL / frec_muestreo_Hz)
#define uart_baud_rate      115200U
#define tamaño_mensaje      32

static const uint8_t ecg_samples[] = {
    17,17,17,17,17,17,17,17,17,17,17,18,18,18,17,17,17,17,17,17,17,18,18,18,18,18,18,18,17,17,16,16,16,16,17,17,18,18,18,17,17,17,17,
    18,18,19,21,22,24,25,26,27,28,29,31,32,33,34,34,35,37,38,37,34,29,24,19,15,14,15,16,17,17,17,16,15,14,13,13,13,13,13,13,13,12,12,
    10,6,2,3,15,43,88,145,199,237,252,242,211,167,117,70,35,16,14,22,32,38,37,32,27,24,24,26,27,28,28,27,28,28,30,31,31,31,32,33,34,36,
    38,39,40,41,42,43,45,47,49,51,53,55,57,60,62,65,68,71,75,79,83,87,92,97,101,106,111,116,121,125,129,133,136,138,139,140,140,139,137,
    133,129,123,117,109,101,92,84,77,70,64,58,52,47,42,39,36,34,31,30,28,27,26,25,25,25,25,25,25,25,25,24,24,24,24,25,25,25,25,25,25,25,
    24,24,24,24,24,24,24,24,23,23,22,22,21,21,21,20,20,20,20,20,19,19,18,18,18,19,19,19,19,18,17,17,18,18,18,18,18,18,18,18,17,17,17,17,
    17,17,17
};
static const size_t ecg_sample_count = sizeof(ecg_samples) / sizeof(ecg_samples[0]);

/*==================[internal data definition]===============================*/
static volatile bool adc_sample_request = false;
static TaskHandle_t uart_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
static void TimerAdcCallback(void *param);
static void transmitir_UART(void *pvParameters);

/*==================[external functions definition]==========================*/

static void TimerAdcCallback(void *param){
    volatile bool *sample_request = (volatile bool *)param;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(sample_request != NULL){
        *sample_request = true;
    }

    if(uart_task_handle != NULL){
        vTaskNotifyGiveFromISR(uart_task_handle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void transmitir_UART(void *pvParameters){
    uint16_t sample_value = 0U;
    char message[tamaño_mensaje];
    size_t sample_index = 0U;

    (void)pvParameters;

    while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if(adc_sample_request){
            adc_sample_request = false;

            AnalogOutputWrite(ecg_samples[sample_index]);
            sample_index = (sample_index + 1U) % ecg_sample_count;

            AnalogInputReadSingle(CH1, &sample_value);
            int len = snprintf(message, sizeof(message), ">brightness:%u\r\n", sample_value);
            if(len > 0){
                UartSendString(UART_PC, message);
            }
        }
    }
}

void app_main(void){
    analog_input_config_t adc_config = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0U,
    };

    serial_config_t uart_config = {
        .port = UART_PC,
        .baud_rate = uart_baud_rate,
        .func_p = UART_NO_INT,
        .param_p = NULL,
    };

    timer_config_t timer_config = {
        .timer = TIMER_A,
        .period = SAMPLE_PERIOD_US,
        .func_p = TimerAdcCallback,
        .param_p = &adc_sample_request,
    };

    UartInit(&uart_config);
    AnalogInputInit(&adc_config);
    AnalogOutputInit();

    xTaskCreate(transmitir_UART, "uart_transmit_task", 4096, NULL, 5, &uart_task_handle);

    TimerInit(&timer_config);
    TimerStart(TIMER_A);

    printf("Prueba del osciloscopio: use doc/otro ecg.txt para la señal ECG analógica y conecte DAC->CH1.\n");

    while(true){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*==================[end of file]============================================*/
