#ifndef MANEJO_IO_H
#define MANEJO_IO_H

#include "planificacion.h"
#include "kernel-conexiones.h"




/**
 * @brief Inicia el receptor de IO
 * 
 * Esta función inicia el receptor de IO, que es el encargado de recibir los modulos IO
 * 
 */
void* iniciar_receptor_io();


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

#endif
