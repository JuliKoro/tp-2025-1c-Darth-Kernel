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
 * @brief Estructura que representa a una entrada de la TLB con toda su informacion
 * 
 * @param pagina Número de página
 * @param marco Numero de Marco correspondiente
 * @param pid ID del Proceso correspondiente
 * @param ingreso Momento en que ingresó a la TLB
 * @param ultimo_uso Momento en que se usó por última vez
 */
typedef struct {
    uint32_t pagina;     // Número de página
    uint32_t marco;      // Marco correspondiente
    uint32_t pid;        // Proceso correspondiente
    uint32_t ingreso;    // Momento en que ingresó a la TLB
    uint32_t ultimo_uso; // Momento en que se usó por última vez
} t_entrada_tlb;

/**
 * @brief Etructura que representa a la TLB (Translation Lookaside Buffer)
 * 
 * @param entradas Arreglo dinámico de entradas de la TLB
 * @param capacidad Capacidad de la TLB (cantidad de entradas)
 * @param tamanio Tamaño actual de la TLB
 * @param algoritmo_reemplazo // Algoritmo de reemplazo (FIFO o LRU)
 */
typedef struct {
    t_entrada_tlb* entradas; // Arreglo dinámico de entradas de la TLB
    int32_t capacidad;     // Capacidad de la TLB
    int32_t tamanio;        // Tamaño actual de la TLB
    char* algoritmo_reemplazo; // Algoritmo de reemplazo (FIFO, LRU)
} tlb_t;

/**
 * @brief Variable Global de la TLB
 */
extern tlb_t* tlb;

/**
 * @brief Traduce una dirección lógica a una dirección física.
 *
 * Esta función calcula el número de página y el desplazamiento a partir de la dirección lógica,
 * consulta la TLB para obtener el marco correspondiente y, si no se encuentra en la TLB, 
 * consulta la memoria para obtener el marco. Finalmente, agrega el marco a la TLB.
 *
 * @param direccion_logica Dirección lógica a traducir.
 * @param pid Identificador del proceso que realiza la traducción.
 * @param socket_memoria Socket utilizado para la comunicación con el módulo de memoria.
 * @return uint32_t Dirección física resultante de la traducción, o -1 en caso de error.
 */
uint32_t traducir_direccion_logica(uint32_t direccion_logica, uint32_t pid, int socket_memoria);

/**
 * @brief Obtiene el marco de memoria correspondiente a una página.
 *
 * @param numero_pagina Número de página para la cual se solicita el marco.
 * @param entradas_niveles Puntero a un arreglo que contiene las entradas de cada nivel.
 * @param pid Identificador del proceso que realiza la solicitud.
 * @param socket_memoria Socket utilizado para la comunicación con el módulo de memoria.
 * @return uint32_t Número de marco obtenido, o -1 en caso de error.
 */
uint32_t obtener_marco_de_memoria(uint32_t numero_pagina, uint32_t* entradas_niveles, uint32_t pid, int socket_memoria);

/**
 * @brief Crea una nueva TLB
 *
 * @return tlb_t* Puntero a la nueva TLB creada.
 * 
 * @note Se destruye con destruir_tlb()
 */
tlb_t* crear_tlb();

/**
 * @brief Destruye la TLB existente.
 *
 * Esta función libera la memoria ocupada por las entradas de la TLB y la estructura
 * de la TLB en sí, evitando punteros colgantes.
 */
void destruir_tlb();

/**
 * @brief Limpia las entradas de la TLB asociadas a un proceso específico.
 *
 * @param pid Identificador del proceso cuyas entradas se desean limpiar.
 */
void limpiar_tlb(uint32_t pid);

/**
 * @brief Verifica si la TLB está habilitada.
 *
 * @return bool Retorna true si la TLB está habilitada y se puede acceder, 
 *               false si la TLB está deshabilitada o no tiene entradas.
 */
bool acceder_tlb();

/**
 * @brief Consulta la TLB para obtener la posición de una entrada.
 *
 * @param pid Identificador del proceso que realiza la consulta.
 * @param numero_pagina Número de página que se desea consultar en la TLB.
 * @return int32_t Posición de la entrada en la TLB, o -1 si no se encuentra.
 */
int32_t consultar_tlb(uint32_t pid, uint32_t numero_pagina);

/**
 * @brief Agrega una entrada a la TLB.
 *
 * Agrega una nueva entrada a la TLB, o reemplaza una entrada existente
 * si la TLB está llena.
 *
 * @param numero_pagina Número de página que se desea agregar a la TLB.
 * @param marco_tlb Número de marco correspondiente a la página.
 * @param pid Identificador del proceso que realiza la operación.
 */
void agregar_a_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid);

/**
 * @brief Reemplaza una entrada en la TLB.
 *
 * @param numero_pagina Número de página que se desea agregar a la TLB.
 * @param marco_tlb Número de marco correspondiente a la página.
 * @param pid Identificador del proceso que realiza la operación.
 */
void reemplazar_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid);

/**
 * @brief Obtiene el número de página a partir de una dirección lógica.
 *
 * @param direccion_logica Dirección lógica de la cual se desea obtener el número de página.
 * @return uint32_t Número de página correspondiente a la dirección lógica.
 */
uint32_t obtener_numero_pagina(uint32_t direccion_logica);

#endif