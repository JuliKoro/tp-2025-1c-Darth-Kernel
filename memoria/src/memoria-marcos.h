#ifndef MEMORIA_MARCOS_H
#define MEMORIA_MARCOS_H

#include "memoria.h"

/**
 * @brief Cabecera para la gestión de marcos de memoria.
 *
 * Declara las funciones relacionadas con la inicialización, asignación
 * y liberación de marcos de memoria, así como la búsqueda de marcos en tablas.
 */

/**
 * @brief Inicializa la lista de marcos libres.
 *
 * Divide la memoria principal en marcos de tamaño `tampagina` y los añade
 * a la lista de `marcos_libres`.
 */
void inicializar_marcos();

/**
 * @brief Obtiene un marco de memoria libre.
 *
 * Primero intenta obtener un marco de la lista de `marcos_libres`.
 * Si no hay marcos libres, aplica el algoritmo FIFO para seleccionar un marco
 * ocupado, lo swapea si es necesario y lo devuelve.
 *
 * @return Puntero al marco de memoria libre u ocupado (si se desalojó), o NULL si no hay marcos disponibles.
 */
void* obtener_marco_libre();

/**
 * @brief Libera un marco de memoria.
 *
 * Elimina el marco de la lista de `marcos_ocupados` y lo añade a la lista de `marcos_libres`.
 * También actualiza la entrada de página que utilizaba este marco para marcarla como no presente.
 *
 * @param marco Puntero al marco de memoria a liberar.
 */
void liberar_marco(void* marco);

/**
 * @brief Busca la entrada de página y el PID asociado a un marco dado.
 *
 * Itera sobre todas las tablas de páginas de todos los procesos para encontrar
 * qué entrada de página está utilizando el `marco_victima` y a qué PID pertenece.
 *
 * @param marco_victima Puntero al marco de memoria a buscar.
 * @return Una estructura `t_marco_info` con el PID, la tabla y la entrada de página
 *         asociados al marco. Si no se encuentra, el PID será -1.
 */
t_marco_info buscar_marco_en_tablas(void* marco_victima);

#endif /* MEMORIA_MARCOS_H */