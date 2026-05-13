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
#define frec_muestreo_Hz    50U  // uso 50 Hz porque el graficador es muy lento para 500 Hz
#define SAMPLE_PERIOD_US    (1000000UL / frec_muestreo_Hz)  
#define uart_baud_rate      115200U  // uso este baudrate en particular porque coincide con un valor de los disponibles en el graficador
#define tamaño_mensaje      32  // tamaño suficiente para que entre el mensaje

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

    (void)pvParameters;

    while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if(adc_sample_request){
            adc_sample_request = false;
            AnalogInputReadSingle(CH1, &sample_value);
            int len = snprintf(message, sizeof(message), ">pote:%u\r\n", sample_value);
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

    xTaskCreate(transmitir_UART, "uart_transmit_task", 4096, NULL, 5, &uart_task_handle);

    TimerInit(&timer_config);
    TimerStart(TIMER_A);

    printf("Prueba del osciloscopio: use doc/otro ecg.txt para la señal ECG analógica y conecte DAC->CH1.\n");

    while(true){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*==================[end of file]============================================*/
