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
#include "cpu-conexiones.h"

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
    uint32_t pagina;     // Número de página
    uint32_t marco;      // Marco correspondiente
    uint32_t pid;        // Proceso correspondiente
    uint32_t ingreso;    // Momento en que ingresó a la TLB
    uint32_t ultimo_uso; // Momento en que se usó por última vez
} t_entrada_tlb;

typedef struct {
    t_entrada_tlb* entradas; // Arreglo dinámico de entradas de la TLB
    int32_t capacidad;     // Capacidad de la TLB
    int32_t tamanio;        // Tamaño actual de la TLB
    char* algoritmo_reemplazo; // Algoritmo de reemplazo (FIFO, LRU, etc.)
} tlb_t;

extern tlb_t* tlb;

uint32_t traducir_direccion_logica(uint32_t direccion_logica, uint32_t pid, int socket_memoria);

uint32_t obtener_numero_pagina(uint32_t direccion_logica);

uint32_t obtener_marco_de_memoria(uint32_t numero_pagina, uint32_t* entradas_niveles, uint32_t pid, int socket_memoria);

tlb_t* crear_tlb(uint32_t capacidad);

void destruir_tlb();

/**
 * @brief Verifica si la TLB está habilitada.
 *
 * @return bool Retorna true si la TLB está habilitada y se puede acceder, 
 *               false si la TLB está deshabilitada o no tiene entradas.
 */
bool acceder_tlb();


int32_t consultar_tlb(uint32_t pid, uint32_t numero_pagina);

void agregar_a_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid);

void reemplazar_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid);

#endif