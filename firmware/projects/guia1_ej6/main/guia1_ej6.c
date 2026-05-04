/*! @mainpage Proyecto 1 Ejercicio 6 - Display Multiplexado con Código BCD
 *
 * @section genDesc General Description
 * 
 * Resuelve el ejercicio 6 del proyecto 1: 
 * 
 *      Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida 
 *      y dos vectores de estructuras del tipo  gpioConf_t. Uno  de estos vectores es igual al 
 *      definido en el punto anterior y el otro vector mapea los puertos con el dígito del LCD 
 *      a donde mostrar un dato:
 * 
 *              Dígito 1 -> GPIO_19
 *              Dígito 2 -> GPIO_18
 *              Dígito 3 -> GPIO_9
 *
 *      La función deberá mostrar por display el valor que recibe. Reutilice las funciones 
 *      creadas en el punto 4 y 5. Realice la documentación de este ejercicio usando Doxygen.
 *
 * @section hardConn Hardware Connection
 *
 * @subsection bcdPins Pines BCD (Segmentos del Display)
 * |    BCD Bit  |   ESP32   |
 * |:-----------:|:----------|
 * |     B0      |  GPIO_20  |
 * |     B1      |  GPIO_21  |
 * |     B2      |  GPIO_22  |
 * |     B3      |  GPIO_23  |
 *
 * @subsection digitPins Pines de Selección de Dígitos
 * |    Dígito   |   ESP32   |
 * |:-----------:|:----------|
 * |   Dígito 1  |  GPIO_19  |
 * |   Dígito 2  |  GPIO_18  |
 * |   Dígito 3  |  GPIO_9   |
 *
 * @note Los pines GPIO se configuran como salidas digitales (OUTPUT).
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
 * @brief Estructura de configuración de GPIO para el mapeo de pines
 * 
 * Esta estructura almacena la información necesaria para configurar y controlar
 * un pin GPIO específico en el ESP32.
 */

typedef struct  // uso typedef para no tener que escribir struct cada vez que me 
                //quiero referir a gpioConf_t (tipo de dato) 
{
	gpio_t pin;			// número de GPIO
	io_t dir;			// input - output (0 o 1)
} gpioConf_t;

/*==================[internal data definition]===============================*/

/**
 * @brief Mapeo de bits BCD a pines GPIO
 * 
 * Arreglo que contiene la configuración de los 4 bits BCD (B0, B1, B2, B3)
 * y sus correspondientes pines GPIO en el ESP32.
 * Estos pines controlan los segmentos individuales del display de 7 segmentos.
 */
gpioConf_t bcdGpioMap[4] = {    //mapear es relacionar cada bit con un pin de GPIO específico
    {GPIO_20, GPIO_OUTPUT},  // b0 -> GPIO_20
    {GPIO_21, GPIO_OUTPUT},  // b1 -> GPIO_21
    {GPIO_22, GPIO_OUTPUT},  // b2 -> GPIO_22
    {GPIO_23, GPIO_OUTPUT}   // b3 -> GPIO_23
};

/**
 * @brief Mapeo de dígitos del display a pines GPIO
 * 
 * Arreglo que contiene la configuración de los 3 dígitos del display
 * de 7 segmentos y sus correspondientes pines GPIO en el ESP32.
 * Estos pines se utilizan para seleccionar cuál dígito se mostrará (multiplexación).
 */
gpioConf_t digitGpioMap[3] = {  // es para controlar qué dígito del display se activa
    {GPIO_19, GPIO_OUTPUT},  /**< Dígito 1 -> GPIO_19 */
    {GPIO_18, GPIO_OUTPUT},  /**< Dígito 2 -> GPIO_18 */
    {GPIO_9,  GPIO_OUTPUT}   /**< Dígito 3 -> GPIO_9 */
};

/*==================[internal functions declaration]=========================*/

/**
 * @brief Convierte un número decimal a su representación en formato BCD (Binary Coded Decimal)
 * 
 * Esta función toma un número decimal y lo convierte a un arreglo donde cada elemento
 * representa un dígito decimal en formato BCD. Por ejemplo, el número 123 se convierte
 * a [1, 2, 3] donde cada elemento está en rango 0-9.
 *
 * @param data          Número decimal a convertir (debe caber en el número de dígitos)
 * @param digits        Cantidad de dígitos esperados en la representación BCD
 * @param bcd_number    Puntero al arreglo de salida que contendrá los dígitos BCD
 *                      (debe tener al menos 'digits' elementos)
 *
 * @return              0 si la conversión fue exitosa
 *                      -1 si bcd_number es NULL o digits es 0
 *                      -2 si el número no cabe en la cantidad de dígitos especificados
 *
 * @note El arreglo bcd_number se inicializa con ceros antes de la conversión.
 * @note Los dígitos se rellenan alineados a la derecha (los ceros iniciales quedan a la izquierda).
 */
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number) //ejercicio 4
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

/**
 * @brief Configura los pines GPIO para representar un valor BCD
 * 
 * Esta función toma un dígito decimal (0-9) y lo convierte a su representación
 * en 4 bits BCD, luego configura los pines GPIO correspondientes (GPIO_20, GPIO_21,
 * GPIO_22, GPIO_23) para que reflejen este valor binario. Cada bit del BCD se mapea
 * a uno de los 4 pines GPIO.
 *
 * @param bcdDigit      Dígito decimal a representar (0-9, o valores BCD hasta 15)
 * @param gpioConfigs   Puntero al arreglo de configuración GPIO que contiene los 4 pines BCD
 *                      (típicamente bcdGpioMap)
 *
 * @note La función inicializa todos los 4 pines GPIO antes de establecer sus estados.
 * @note Cada bit de bcdDigit (b0-b3) se mapea directamente a gpioConfigs[0-3].
 * @note Los pines se configuran como GPIO_OUTPUT.
 *
 */
void setBCDToGPIOs(uint8_t bcdDigit, gpioConf_t *gpioConfigs) //ejercicio 5
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
/**
 * @brief Muestra un valor numérico en un display multiplexado de 7 segmentos
 * 
 * Esta función implementa el algoritmo de multiplexación de display. Convierte el valor
 * decimal a BCD, y luego itera sobre cada dígito del display, estableciendo los valores
 * BCD apropiados en los pines GPIO y activando el dígito correspondiente.
 *
 * @param data          Valor decimal a mostrar en el display (ej: 123, 45, 7)
 * @param digits        Número de dígitos del display (máximo 3)
 * @param mapBcd        Puntero al arreglo de configuración de los 4 pines BCD
 *                      (típicamente bcdGpioMap)
 * @param mapDigits     Puntero al arreglo de configuración de los pines de selección
 *                      de dígitos (típicamente digitGpioMap)
 *
 * @note La función retorna inmediatamente si digits es 0 o mayor a 3.
 * @note El valor debe caber en la cantidad de dígitos especificados (ej: valor 123 requiere 3 dígitos).
 * @note Para un funcionamiento visual adecuado en el display físico, esta función debe
 *       ser llamada repetidamente en un bucle rápido (efecto de multiplexación).
 * @note La función inicializa todos los pines GPIO necesarios.
 *
 */
void displayValueOnDigits(uint32_t data, uint8_t digits, gpioConf_t *mapBcd, gpioConf_t *mapDigits)
{
    if (digits == 0 || digits > 3) {
        return;
    }

    for (int d = 0; d < digits; d++) {
        GPIOInit(mapDigits[d].pin, mapDigits[d].dir);  // inicializa los GPIO como salida
        GPIOState(mapDigits[d].pin, 0);  // apaga todos los GPIO de selección de dígitos
    }

    uint8_t bcd_digits[3] = {0};
    if (convertToBcdArray(data, digits, bcd_digits) != 0) { //convierte a BCD
        return; // error en conversión
    }

    // Para cada ciclo de refresco, mostramos un dígito a la vez
    for (uint8_t pos = 0; pos < digits; pos++) {
        uint8_t digitValue = bcd_digits[pos];

        setBCDToGPIOs(digitValue, mapBcd);         // Seteo BCD del dígito actual
        
        GPIOState(mapDigits[pos].pin, true);       // Activar el dígito correspondiente del display

        GPIOState(mapDigits[pos].pin, false);      // Apagar el dígito
    }
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación
 * 
 * Punto de entrada del programa. Configura el valor a mostrar (123) en un display
 * multiplexado de 3 dígitos y lo visualiza usando código BCD para el control
 * de los segmentos individuales del display de 7 segmentos.
 *
 * @note El programa entra en un bucle infinito después de mostrar el valor.
 *
 */
void app_main(void)
{
    uint32_t value = 123;      /**< Valor decimal a mostrar en el display */
    uint8_t digits = 3;        /**< Número de dígitos del display */
    
    displayValueOnDigits(value, digits, bcdGpioMap, digitGpioMap);
	while (1) {
    }  // para evitar que el programa llegue a la última llave y se reinicie
}

/*==================[end of file]============================================*/