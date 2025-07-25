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

// Estructura para representar una entrada en la caché
typedef struct {
    uint32_t pagina;      // Número de página
    void* contenido;      // Contenido de la página (puede ser un puntero a datos)
    bool modificado;      // Indica si la página ha sido modificada (Bit de Modificado)
    bool uso;        // Indica si la página ha sido utilizada (Bit de Uso)
} entrada_cache;

// Estructura para la caché de páginas
typedef struct {
    entrada_cache* entradas; // Arreglo de entradas de la caché
    int capacidad;           // Capacidad de la caché
    int tamaño;              // Tamaño actual de la caché
    char* algoritmo_reemplazo; // Algoritmo de reemplazo (CLOCK o CLOCK-M)
    int puntero;            // Puntero para el algoritmo
} cache_t;

extern cache_t* cache; // Variable global de caché

// Prototipos de funciones
cache_t* crear_cache(int capacidad, char* algoritmo_reemplazo);
void destruir_cache();
bool acceder_cache();
bool acceder_pagina_cache(uint32_t pagina, uint32_t pid);
void agregar_a_cache(uint32_t pagina, void* contenido, uint32_t pid);
void actualizar_cache_a_memoria(uint32_t pid, int socket_memoria);
void reemplazar_pagina(uint32_t pagina, uint32_t pid);
void escribir_en_cache(uint32_t direccion_logica, const char* datos, uint32_t pid, int socket_memoria);
char* leer_de_cache(uint32_t direccion_logica, uint32_t tamanio, uint32_t pid, int socket_memoria);
void limpiar_cache()

#endif
