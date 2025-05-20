#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "planificacion.h"


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                funciones auxiliares para las syscalls

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Busca un IO por su nombre
 * 
 * @param nombre_buscado: El nombre del IO a buscar
 * @return El IO encontrado o NULL si no existe
 */
t_io* buscar_io_por_nombre(char* nombre_buscado);





/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                    Syscalls como tal

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/**
 * @brief Inicializa un proceso. Esta syscall se llama cuando CPU ejecute la instruccion INIT_PROC
 * 
 * @param archivo_pseudocodigo: El archivo de pseudocódigo del proceso
 * @param tamanio_proceso: El tamaño del proceso
 */
void init_proc(char* archivo_pseudocodigo, int tamanio_proceso);


/**
 * @brief Ejecuta la syscall de IO
 * 
 * @param nombre_io: El nombre del IO
 * @param tiempo_io: El tiempo de IO en milisegundos
 */
void io (char* nombre_io, int tiempo_io);




#endif