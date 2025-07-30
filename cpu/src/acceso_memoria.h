#ifndef ACCESO_MEMORIA_H
#define ACCESO_MEMORIA_H

#include "cpu-conexiones.h"

/**
 * @brief Lee datos de la memoria a partir de una dirección física.
 *
 * @param pid Identificador del proceso que solicita la lectura de datos.
 * @param direccion_fisica Dirección física desde la cual se desea leer los datos.
 * @param tamanio Tamaño del bloque de datos a leer (para cargar a cache tamanio_pagina)
 * @param socket_memoria Socket utilizado para la comunicación con el módulo de memoria.
 * 
 * @return void* Puntero a los datos leídos de la memoria. Si ocurre un error
 *                durante la operación, se retorna NULL.
 */
void* leer_de_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio, int socket_memoria);

/**
 * @brief Escribe datos en la memoria.
 *
 * @param pid Identificador del proceso que realiza la escritura.
 * @param direccion_fisica Dirección física donde se desea escribir los datos.
 * @param tamanio Tamaño de los datos a escribir.
 * @param dato Puntero a los datos que se desean escribir en memoria.
 * @param socket_memoria Socket utilizado para la comunicación con el módulo de memoria.
 *
 * @return int Retorna 0 si la escritura fue exitosa, -1 en caso de error.
 */
int escribir_en_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio, void* dato, int socket_memoria);

#endif