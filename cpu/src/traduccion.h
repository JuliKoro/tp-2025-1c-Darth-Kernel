#ifndef TRADUCCION_H_
#define TRADUCCION_H_

#include <math.h>

#include "cpu-configs.h"

//Estructura de dirección física
/**
 * @struct t_direccion_fisica
 * @brief Estructura que representa una dirección física
 * 
 * @param nro_pagina: Número de página
 * @param entrada_nivel_X: Entrada nivel X
 * @param desplazamiento: Desplazamiento

 */

typedef struct {
    uint32_t nro_pagina;
    uint32_t desplazamiento;
    uint32_t entrada_nivel_X;
} t_direccion_fisica;

//Estructura de la TLB
/**
 * @struct tlb
 * @brief Estructura que representa la TLB
 * 
 * @param pagina: Número de página
 * @param marco: Marco

 */

typedef struct {
    int pagina;
    uint32_t marco;
} tlb;

/**
* @brief Funcion que traduce una dirección lógica a una física
* @param direccion_logica Dirección lógica a traducir
* @return Dirección física traducida
*/
t_direccion_fisica direccion_logica_a_fisica(uint32_t direccion_logica);

#endif