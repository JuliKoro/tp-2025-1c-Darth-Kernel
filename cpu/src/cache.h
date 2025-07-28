#ifndef CACHE_H
#define CACHE_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "commons/collections/list.h"

#include "cpu-configs.h"
#include "cpu-log.h"
#include "cpu-conexiones.h"
#include "traduccion.h"

/**
 * @brief Estructura para representar una entrada en la caché.
 * 
 * @param pagina Número de página.
 * @param contenido Contenido de la página (puede ser un puntero a datos).
 * @param modificado Indica si la página ha sido modificada (Bit de Modificado).
 * @param uso Indica si la página ha sido utilizada (Bit de Uso).
 */
typedef struct {
    uint32_t pagina;      // Número de página.
    void* contenido;      // Contenido de la página (un puntero a datos).
    bool modificado;      // Indica si la página ha sido modificada (Bit de Modificado).
    bool uso;            // Indica si la página ha sido utilizada (Bit de Uso).
} entrada_cache;

/**
 * @brief Estructura para la caché de páginas.
 *
 * @param entradas Arreglo de entradas de la caché
 * @param capacidad Capacidad de la caché
 * @param tamanio Tamaño actual de la caché
 * @param algoritmo_reemplazo Algoritmo de reemplazo (CLOCK o CLOCK-M)
 * @param puntero Puntero para el algoritmo
 */
typedef struct {
    entrada_cache* entradas; // Arreglo de entradas de la caché.
    int capacidad;           // Capacidad de la caché.
    int tamanio;              // Tamaño actual de la caché.
    char* algoritmo_reemplazo; // Algoritmo de reemplazo (CLOCK o CLOCK-M).
    int puntero;            // Puntero para el algoritmo.
} cache_t;

/**
 * @brief Variable global que representa la caché de páginas.
 */
extern cache_t* cache; // Variable global de caché.

/**
 * @brief Crea una nueva caché de páginas.
 *
 * @param capacidad Capacidad máxima de la caché.
 * @param algoritmo_reemplazo Algoritmo de reemplazo a utilizar (ej. "CLOCK" o "CLOCK-M").
 *
 * @return cache_t* Puntero a la nueva caché de páginas creada.
 * 
 * @note Hay que destruirla con destruir_cache(cache_t* cache), ya que se asigna memoria y hay que liberarla
 */
cache_t* crear_cache(int capacidad, char* algoritmo_reemplazo);

/**
 * @brief Destruye la caché de páginas.
 *
 * Libera la memoria asignada para la caché de páginas y sus entradas.
 * Se debe llamar cuando la caché ya no es necesaria.
 *
 */
void destruir_cache();

/**
 * @brief Verifica si la caché está habilitada.
 *
 * @return bool Retorna true si la caché está habilitada, false en caso contrario.
 */
bool acceder_cache();

/**
 * @brief Accede a una página en la caché.
 *
 * Verifica si una página específica está presente en la caché.
 *
 * @param pagina Número de la página a buscar.
 * @param pid Identificador del proceso que realiza la búsqueda.
 *
 * @return bool Retorna true si la página se encuentra en la caché (Cache Hit),
 *              false si no se encuentra (Cache Miss).
 */
bool acceder_pagina_cache(uint32_t pagina, uint32_t pid);

/**
 * @brief Carga una página en la caché desde el módulo de memoria.
 *
 * @param pagina Número de la página que se desea cargar en la caché.
 * @param pid Identificador del proceso que solicita la carga de la página.
 * @param socket_memoria Socket utilizado para la comunicación con el módulo de memoria.
 */
void cargar_pagina_en_cache(uint32_t pagina, uint32_t pid, int socket_memoria);

/**
 * @brief Agrega una página a la caché.
 *
 * Agrega una nueva página a la caché. Si la caché está llena,
 * se reemplaza una página existente según el algoritmo de reemplazo.
 *
 * @param pagina Número de la página a agregar.
 * @param contenido Puntero al contenido de la página.
 * @param pid Identificador del proceso que realiza la operación.
 */
void agregar_a_cache(uint32_t pagina, void* contenido, uint32_t pid);

/**
 * @brief Actualiza la caché en memoria.
 *
 * Recorre las entradas de la caché y actualiza en memoria
 * aquellas que han sido modificadas.
 *
 * @param pid Identificador del proceso que realiza la operación.
 * @param socket_memoria Socket para la comunicación con la memoria.
 */
void actualizar_cache_a_memoria(uint32_t pid, int socket_memoria);

/**
 * @brief Reemplaza una página en la caché.
 *
 * Reemplaza una página existente en la caché utilizando el
 * algoritmo de reemplazo especificado (CLOCK o CLOCK-M).
 *
 * @param pagina Número de la nueva página a agregar.
 * @param pid Identificador del proceso que realiza la operación.
 */
void reemplazar_pagina(uint32_t pagina, uint32_t pid);

/**
 * @brief Escribe datos en la caché.
 *
 * Escribe datos en la caché para una dirección lógica dada.
 * Si la página no está en la caché, se carga desde la memoria.
 *
 * @param direccion_logica Dirección lógica donde se desea escribir.
 * @param datos Datos a escribir en la caché.
 * @param pid Identificador del proceso que realiza la operación.
 * @param socket_memoria Socket para la comunicación con la memoria.
 */
void escribir_en_cache(uint32_t direccion_logica, const char* datos, uint32_t pid, int socket_memoria);

/**
 * @brief Lee datos de la caché.
 *
 * Lee datos de la caché para una dirección lógica dada.
 * Si la página no está en la caché, se carga desde la memoria.
 *
 * @param direccion_logica Dirección lógica de donde se desea leer.
 * @param tamanio Tamaño de los datos a leer.
 * @param pid Identificador del proceso que realiza la operación.
 * @param socket_memoria Socket para la comunicación con la memoria.
 *
 * @return char* Puntero a los datos leídos de la caché o de la memoria.
 */
char* leer_de_cache(uint32_t direccion_logica, uint32_t tamanio, uint32_t pid, int socket_memoria);

/**
 * @brief Limpia la caché de páginas.
 *
 * Libera la memoria asignada a las entradas de la caché y
 * restablece todos los campos a sus valores iniciales.
 *
 * @return void No retorna ningún valor.
 */
void limpiar_cache();

#endif
