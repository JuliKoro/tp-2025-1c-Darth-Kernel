#ifndef MEMORIA_SWAP_H
#define MEMORIA_SWAP_H

#include "memoria.h"

/**
 * @brief Cabecera para la gestión del espacio SWAP.
 *
 * Declara las funciones relacionadas con la inicialización del archivo SWAP,
 * gestión de posiciones libres, y operaciones de lectura/escritura en SWAP.
 */

/**
 * @brief Inicializa el archivo de SWAP y su bitmap.
 *
 * Abre o crea el archivo especificado en `PATH_SWAPFILE`. Si es nuevo, lo llena
 * con ceros. Calcula el número total de páginas en SWAP y crea un `t_bitarray`
 * para gestionar las posiciones libres/ocupadas.
 */
void inicializar_swap();

/**
 * @brief Obtiene la primera posición libre en el archivo SWAP.
 *
 * Busca el primer bit en 0 en el `bitmap_swap`, lo marca como 1 (ocupado)
 * y devuelve su índice.
 *
 * @return Índice de la posición libre, o -1 si no hay espacio.
 */
int obtener_posicion_libre_swap();

/**
 * @brief Libera una posición en el archivo SWAP.
 *
 * Marca el bit correspondiente a la `posicion` en el `bitmap_swap` como 0 (libre).
 *
 * @param posicion Índice de la posición a liberar.
 */
void liberar_posicion_swap(int posicion);

/**
 * @brief Escribe una página de datos en el archivo SWAP.
 *
 * Escribe el contenido de `pagina` en la `posicion` especificada dentro
 * del archivo SWAP.
 *
 * @param posicion Índice de la posición en SWAP donde escribir.
 * @param pagina Puntero a los datos de la página a escribir.
 */
void escribir_pagina_swap(int posicion, void* pagina);

/**
 * @brief Lee una página de datos desde el archivo SWAP.
 *
 * Lee el contenido de la `posicion` especificada en el archivo SWAP
 * y lo copia a `destino`.
 *
 * @param posicion Índice de la posición en SWAP desde donde leer.
 * @param destino Puntero al buffer donde se copiarán los datos leídos.
 */
void leer_pagina_swap(int posicion, void* destino);

/**
 * @brief Aplica un retardo basado en `RETARDO_SWAP`.
 *
 * Utiliza `usleep` para pausar la ejecución por el tiempo configurado
 * en milisegundos.
 */
void aplicar_retardo_swap();

#endif /* MEMORIA_SWAP_H */