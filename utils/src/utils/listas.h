#ifndef LISTAS_H
#define LISTAS_H

#include <commons/collections/list.h>
#include <stddef.h>

/**
 * @brief Busca un elemento en una lista con un parametro
 * La funcion de comparacion debe ser una funcion que reciba un void* elemento y un void* cosa a comparar
 * 
 * @param lista Lista en la que se busca
 * @param comparar Función de comparación
 * @param param Parametro de la función de comparación
 */
void* list_find_con_param(t_list* lista, bool (*comparar)(void*, void*), void* param);

#endif
