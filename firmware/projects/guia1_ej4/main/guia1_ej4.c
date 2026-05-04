/*! @mainpage Proyecto 1 Ejercicio 4 -Conversión de enteros a arreglo de dígitos (BCD).

 * @section genDesc General Description
 * 
 * Resuelve el ejercicio 4 del proyecto 1: 
 * 
 *      Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida 
 *      y un puntero a un arreglo donde se almacene los n dígitos. La función deberá convertir 
 *      el dato recibido a BCD, guardando cada uno de los dígitos de salida en el arreglo 
 *      pasado como puntero.
 *
 * @author Ana Clara Polari (ana.polari@ingenieria.uner.edu.ar)
 *
 */
/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/

int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number) // los parámetros son el número a convertir,
                                                                                //la cantidad de dígitos y un puntero al lugar donde
                                                                                // se va a guardar 
{
    if (bcd_number == NULL || digits == 0) {
        return -1; // parámetros inválidos, devuelve -1
    }
    
    for (uint8_t i = 0; i < digits; i++)  {  // inicialización: llenar con ceros
        bcd_number[i] = 0;
    }

    for (int i = digits - 1; i >= 0; i--){  // se llena de derecha a izquierda (digito menos significativo primero)
        bcd_number[i] = data % 10; //obtiene el resto de dividir por 10 (último digito)
        data /= 10; // "saca" el último dígito del número, división entera
        if (data == 0) {
            break;
        }
    }
    
    if (data != 0){  // si queda data > 0, entonces no entró en el arreglo (overflow de dígitos)
        return -2;
    }

    return 0; // éxito
}

void app_main(void){
    uint8_t bcd_array[5]; // vector para almacenar en memoria

    int8_t result = convertToBcdArray(12345, 5, bcd_array); // Ejemplo: convertir 12345 a BCD con 5 dígitos

    if(result == 0){
        for(uint8_t i = 0; i < 5; i++){
            printf("%d ", bcd_array[i]);
        }
        printf("\n");
    } else {
        printf("Error en la conversión: %d\n", result); // se va a mostrar -1 o -2 según cual sea el error
    }
}

/*==================[end of file]============================================*/