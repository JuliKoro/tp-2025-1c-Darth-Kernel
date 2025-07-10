#ifndef MEMORIA_ADMIN_H
#define MEMORIA_ADMIN_H

#include "memoria.h"

/**
 * @brief Cabecera para la gestión del administrador de memoria.
 *
 * Declara las funciones para inicializar y destruir el administrador de memoria,
 * que es responsable de gestionar toda la memoria del sistema.
 */

/**
 * @brief Inicializa el administrador de memoria.
 *
 * Reserva memoria para la estructura `t_administrador_memoria` y para la memoria principal.
 * Inicializa las listas de marcos libres y ocupados, los diccionarios de tablas de páginas
 * y métricas, y llama a las funciones de inicialización de marcos y SWAP.
 */
void inicializar_administrador_memoria();

/**
 * @brief Destruye todas las estructuras del administrador de memoria.
 *
 * Libera la memoria principal, las listas de marcos, los diccionarios
 * de tablas de páginas y métricas, y cierra el archivo SWAP.
 */
void destruir_administrador_memoria();

int obtener_posicion_libre_swap();

//
void escribir_pagina_swap(int posicion, void* pagina);

//
void leer_pagina_swap(int posicion, void* destino);

#endif /* MEMORIA_ADMIN_H */