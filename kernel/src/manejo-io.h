#ifndef MANEJO_IO_H
#define MANEJO_IO_H

#include "planificacion.h"
#include "kernel-conexiones.h"
#include <kernel-log.h>
#include <utils/listas.h>
#include <utils/serializacion.h>


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
 * Recibe un socket, recibe el nombre del IO. Si ya existe en la lista, aumenta el numero de instancias. Si no existe, lo agrega a la lista.
 * Despues de agregar el IO a la lista, crea un hilo de atencion para la IO. (funcion atender_io)
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

/**
 * @brief Itera por la lista de IO y muestra cada IO
 * 
 * Esta función muestra la lista de IO
 * 
 */
void mostrar_lista_io();

/**
 * @brief Recibe el nombre un IO, lo busca en la lista, y aumenta el numero de instancias que dispone
 * 
 * @param nombre_io Nombre del IO a aumentar
 * 
 */
void aumentar_instancias_disponibles(char* nombre_io);

/**
 * @brief Atiende una IO. Consiste en un loop infinito que comprueba que la conexion siga abierta, si la conexion falla, elimina la instancia de la lista
 * 
 * @param socket_ptr Puntero al socket de la conexion
 * 
 */
void* atender_io(void* socket_ptr);


/**
 * @brief Elimina una instancia de IO de la lista. Si esa era la ultima instancia, tambien elimina la IO de la lista.
 * 
 * @param socket_io Socket de la instancia de IO a eliminar
 * 
 */
void eliminar_instancia_io(int socket_io);

#endif
