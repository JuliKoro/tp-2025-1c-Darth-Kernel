#ifndef CPU_CONEXIONES_H_
#define CPU_CONEXIONES_H_


#include <utils/sockets.h>

#include "cpu-configs.h"

/**
* @brief Conecta con el kernel
* @return Devuelve socket del cliente para comunicarse con kernel
*/
int cpu_conectar_a_kernel(void);

/**
* @brief Conecta con la memoria
* @return Devuelve socket del cliente para comunicarse con memoria
*/
int cpu_conectar_a_memoria(void);


#endif