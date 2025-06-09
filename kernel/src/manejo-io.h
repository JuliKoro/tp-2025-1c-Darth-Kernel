#ifndef MANEJO_IO_H
#define MANEJO_IO_H

#include "planificacion.h"
#include "kernel-conexiones.h"
#include <kernel-log.h>
#include <utils/listas.h>



/**
 * @brief Inicia el receptor de IO
 * 
 * Esta función inicia el receptor de IO, que es el encargado de recibir los modulos IO
 * 
 */
void* iniciar_receptor_io();

/**
 * @brief Compara el nombre de un IO
 * 
 * Esta función compara el nombre de un IO con el nombre de un IO
 * 
 */
bool comparar_nombre_io(void* elemento, void* nombre_a_comparar);

/**
 * @brief Guarda el IO en la lista de IO
 * 
 * Esta función guarda el IO en la lista de IO
 * 
 */
void* guardar_io(void* socket_ptr);




/**
 * @brief Agrega el IO a la lista de IO
 * 
 * Esta función agrega el IO a la lista de IO
 * 
 */
void agregar_io_a_lista(t_io* io);

void mostrar_lista_io();

void aumentar_instancias_disponibles(char* nombre_io);

#endif
