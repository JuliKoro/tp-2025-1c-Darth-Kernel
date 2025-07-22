#ifndef TRADUCCION_H_
#define TRADUCCION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "commons/collections/list.h"
#include "commons/collections/queue.h"

#include "utils/serializacion.h"

#include "cpu-configs.h"
#include "cpu-log.h"

/**
 * @struct t_direccion_logica
 * @brief Estructura que represeta a la direccion logica por partes
 * 
 * @param 
 */
typedef struct {
    uint32_t* entradas_niveles; // Arreglo dinámico para las entradas de los niveles
    uint32_t desplazamiento;     // Desplazamiento
    uint32_t cant_niveles;       // Cantidad de niveles
} t_direccion_logica;

typedef struct {
    uint32_t pagina; // Número de página
    uint32_t marco;  // Marco correspondiente
    uint32_t pid;    // Proceso correspondiente
} t_entrada_tlb;

typedef struct {
    t_entrada_tlb* entradas; // Arreglo dinámico de entradas de la TLB
    int32_t capacidad;     // Capacidad de la TLB
    int32_t tamaño;        // Tamaño actual de la TLB
    char* algoritmo_reemplazo; // Algoritmo de reemplazo (FIFO, LRU, etc.)
} tlb_t;

extern t_tabla_pag* info_tabla_pag;
extern tlb_t* tlb;

/*
//Estructura de dirección física
/**
 * @struct t_direccion_fisica
 * @brief Estructura que representa una dirección física
 * 
 * @param nro_pagina: Número de página
 * @param entrada_nivel_X: Entrada nivel X
 * @param desplazamiento: Desplazamiento
 */
/*
 typedef struct {
    uint32_t nro_pagina;
    uint32_t desplazamiento;
    uint32_t entrada_nivel_X;
} t_direccion_fisica;
*/
/*
//Estructura de la TLB
/**
 * @struct tlb
 * @brief Estructura que representa la TLB
 * 
 * @param pagina: Número de página
 * @param marco: Marco
 */
/*
 typedef struct {
    int pagina;
    uint32_t marco;
} tlb;
*/

/*
/**
* @brief Funcion que traduce una dirección lógica a una física
* @param direccion_logica Dirección lógica a traducir
* @return Dirección física traducida
*/
/*
t_direccion_fisica direccion_logica_a_fisica(uint32_t direccion_logica);
*/

uint32_t traducir_direccion_logica(uint32_t direccion_logica, uint32_t pid, int socket_memoria);

uint32_t obtener_marco_de_memoria(uint32_t numero_pagina, int socket_memoria, uint32_t pid);

tlb_t* crear_tlb(uint32_t capacidad);

void destruir_tlb();

int32_t consultar_tlb(uint32_t pid, uint32_t numero_pagina);

void agregar_a_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid);

void reemplazar_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid);

#endif