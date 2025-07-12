#ifndef CPU_CONEXIONES_H_
#define CPU_CONEXIONES_H_


#include <utils/sockets.h>

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

#endif