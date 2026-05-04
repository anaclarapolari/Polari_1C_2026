/*! @mainpage Proyecto 1 Ejercicio 5 - Decodificador de BCD a Salidas Digitales (GPIO Mapping).
 *
 * @section genDesc General Description
 * 
 * Resuelve el ejercicio 5 del proyecto 1: 
 * 
 *      Escribir una función que reciba como parámetro un dígito BCD y un vector de estructuras
 *      del tipo gpioConf_t. Incluya el archivo de cabecera gpio_mcu.h
 *      Defina un vector que mapee los bits de la siguiente manera:
 * 
 *             b0 -> GPIO_20
 *             b1 -> GPIO_21
 *             b2 -> GPIO_22
 *             b3 -> GPIO_23
 * 
 *      La función deberá cambiar el estado de cada GPIO, a ‘0’ o a ‘1’, según el estado del 
 *      bit correspondiente en el BCD ingresado. Ejemplo: b0 se encuentra en ‘1’, el estado 
 *      de GPIO_20 debe setearse. 
 *
 * @author Ana Clara Polari (ana.polari@ingenieria.uner.edu.ar)
 *
 */
/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

typedef struct  // uso typedef para no tener que escribir struct cada vez que me 
                //quiero referir a gpioConf_t (tipo de dato) 
{
	gpio_t pin;			// número de GPIO
	io_t dir;			// input - output (0 o 1)
} gpioConf_t;

/*==================[internal data definition]===============================*/

gpioConf_t bcdGpioMap[4] = {    //mapear es relacionar cada bit con un pin de GPIO específico
    {GPIO_20, GPIO_OUTPUT},  // b0 -> GPIO_20
    {GPIO_21, GPIO_OUTPUT},  // b1 -> GPIO_21
    {GPIO_22, GPIO_OUTPUT},  // b2 -> GPIO_22
    {GPIO_23, GPIO_OUTPUT}   // b3 -> GPIO_23
};

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/

void setBCDToGPIOs(uint8_t bcdDigit, gpioConf_t *gpioConfigs) // recibe un dígito BCD y un vector de struct gpioConf_t
{

    for (int i = 0; i < 4; i++) {
        GPIOInit(gpioConfigs[i].pin, gpioConfigs[i].dir); // inicializa los GPIO como salidas
    }

    for (int i = 0; i < 4; i++) {      //hasta 4, porque un dígito BCD tiene 4 bits
        
        bool bitState = (bcdDigit & (1 << i)) != 0;  // la máscara desplaza el 1 a la izquierda en cada ciclo, 
                                                    // cuando coincide con bcdDigit, devuelve 1
        GPIOState(gpioConfigs[i].pin, bitState);     // setea CADA GPIO según el 0 o 1 que obtuvo antes
    }
}

void app_main(void)
{
    uint8_t bcdDigit = 5;  // b0=1, b1=0, b2=1, b3=0
    setBCDToGPIOs(bcdDigit, bcdGpioMap);
}

/*==================[end of file]============================================*/