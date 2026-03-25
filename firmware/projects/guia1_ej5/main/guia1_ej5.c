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
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal data definition]===============================*/

// Vector que mapea los bits BCD a GPIOs
gpioConf_t bcdGpioMap[4] = {
    {GPIO_20, GPIO_OUTPUT},  // b0 -> GPIO_20
    {GPIO_21, GPIO_OUTPUT},  // b1 -> GPIO_21
    {GPIO_22, GPIO_OUTPUT},  // b2 -> GPIO_22
    {GPIO_23, GPIO_OUTPUT}   // b3 -> GPIO_23
};

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/

// Recibe un dígito BCD y un vector de gpioConf_t
void setBCDToGPIOs(uint8_t bcdDigit, gpioConf_t *gpioConfigs)
{
    // Inicializar los GPIOs como salidas
    for (int i = 0; i < 4; i++) {
        GPIOInit(gpioConfigs[i].pin, gpioConfigs[i].dir);
    }

    // Para cada bit del dígito BCD, determinar su estado usando operaciones lógicas
    for (int i = 0; i < 4; i++) {
        // Usar máscara y shift para extraer el bit i (multiplicación lógica con 1 << i)
        bool bitState = (bcdDigit & (1 << i)) != 0;  // 1 si el bit está en 1, 0 si en 0
        GPIOState(gpioConfigs[i].pin, bitState);     // Setear el GPIO al estado del bit
    }
}

void app_main(void)
{
    uint8_t bcdDigit = 5;  // 5 en BCD: bits b0=1, b1=0, b2=1, b3=0
    setBCDToGPIOs(bcdDigit, bcdGpioMap);
    printf("BCD %d configurado en GPIOs.\n", bcdDigit);
}
/*==================[end of file]============================================*/