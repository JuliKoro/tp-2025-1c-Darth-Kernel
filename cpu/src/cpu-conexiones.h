#ifndef CPU_CONEXIONES_H_
#define CPU_CONEXIONES_H_

#include <utils/sockets.h>
#include <utils/serializacion.h>

#include "cpu-configs.h"
#include "cpu-log.h"
/**
* @brief Establece la conexión entre el módulo CPU y el módulo Kernel.
* @param puerto_kernel dispatch o interrupt
* @param id_cpu ID del cpu pasado como argumento al ejecutar
* @return int - Devuelve el socket del cliente para comunicarse con kernel, -1 en caso de error
*/
int cpu_conectar_a_kernel(int puerto_kernel, int id_cpu);

/**
* @brief Establece la conexión entre el módulo CPU y el módulo Memoria.
* @param id_cpu ID de la CPU (solo para logger)
* @return int - Devuelve el socket del cliente para comunicarse con memoria, -1 en caso de error
*/
int cpu_conectar_a_memoria(int id_cpu);

/**
 * @brief Realiza el handshake entre la CPU y la memoria.
 *
 * @param socket_memoria Socket a través del cual se comunica la CPU con la memoria.
 * @param id_cpu Identificador único de la CPU que está realizando el handshake.
 *
 * @return t_tabla_pag* Puntero a una estructura de tipo t_tabla_pag que contiene la información
 *                       de la tabla de páginas recibida de la memoria. Si ocurre un error
 *                       durante el proceso, se retorna NULL.
 * 
 * @note CPU envia su id_cpu (int), memoria verifica (numero entero positivo) y responde enviando un paquete con t_tabla_pag
 */
t_tabla_pag* hanshake_cpu_memoria(int socket_memoria, int id_cpu);


#endif