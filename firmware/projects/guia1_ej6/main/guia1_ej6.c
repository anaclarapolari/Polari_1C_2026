/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
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
#include "gpio_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/
/**
 * @brief GPIO configuration structure
 */
typedef struct
{
    gpio_t pin;      /*!< GPIO pin number */
    io_t dir;        /*!< GPIO direction '0' IN; '1' OUT */
} gpioConf_t;

/*==================[internal data definition]===============================*/
/**
 * @brief Mapeo BCD a GPIO (ejercicio 5)
 */
gpioConf_t bcdGpioMap[4] = {
    {GPIO_20, GPIO_OUTPUT},  // b0 -> GPIO_20
    {GPIO_21, GPIO_OUTPUT},  // b1 -> GPIO_21
    {GPIO_22, GPIO_OUTPUT},  // b2 -> GPIO_22
    {GPIO_23, GPIO_OUTPUT}   // b3 -> GPIO_23
};
/**
 * @brief Mapeo de dígitos de display a GPIO (ejercicio actual)
 */
gpioConf_t digitGpioMap[3] = {
    {GPIO_19, GPIO_OUTPUT},  // Dígito 1
    {GPIO_18, GPIO_OUTPUT},  // Dígito 2
    {GPIO_9,  GPIO_OUTPUT}   // Dígito 3
};

/*==================[internal functions declaration]=========================*/

int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
    if (bcd_number == NULL || digits == 0) {
        return -1;
    }
    for (uint8_t i = 0; i < digits; i++) {
        bcd_number[i] = 0;
    }
    for (int i = digits - 1; i >= 0; i--) {
        bcd_number[i] = data % 10;
        data /= 10;
        if (data == 0) {
            break;
        }
    }
    if (data != 0) {
        return -2;
    }
    return 0;
}

void setBCDToGPIOs(uint8_t bcdDigit, gpioConf_t *gpioConfigs)
{
    for (int i = 0; i < 4; i++) {
        GPIOInit(gpioConfigs[i].pin, gpioConfigs[i].dir);
    }
    for (int i = 0; i < 4; i++) {
        bool bitState = (bcdDigit & (1 << i)) != 0;
        GPIOState(gpioConfigs[i].pin, bitState);
    }
}

void displayValueOnDigits(uint32_t data, uint8_t digits, gpioConf_t *mapBcd, gpioConf_t *mapDigits)
{
    if (digits == 0 || digits > 3) {
        return;
    }
    // Inicializa los GPIOs de dígito (desactivados inicialmente)
    for (int d = 0; d < digits; d++) {
        GPIOInit(mapDigits[d].pin, mapDigits[d].dir);
        GPIOState(mapDigits[d].pin, 0);
    }

    // Convertir data a arreglo BCD
    uint8_t bcd_digits[3] = {0};
    if (convertToBcdArray(data, digits, bcd_digits) != 0) {
        return; // Error en conversión
    }

    // Para cada ciclo de refresco, mostramos un dígito a la vez
    for (uint8_t pos = 0; pos < digits; pos++) {
        uint8_t digitValue = bcd_digits[pos];

        // Seteo BCD del dígito actual
        setBCDToGPIOs(digitValue, mapBcd);

        // Activar el dígito correspondiente del display
        GPIOState(mapDigits[pos].pin, true);

        // Apagar el dígito previo
        GPIOState(mapDigits[pos].pin, false);
    }
}

/*==================[external functions definition]==========================*/

void app_main(void)
{
    uint32_t value = 123;
    uint8_t digits = 3;

    // printf("Mostrando %u en display (3 digitos)\n", value);
    
    displayValueOnDigits(value, digits, bcdGpioMap, digitGpioMap);

	while (1) {
    }
}

/*==================[end of file]============================================*/