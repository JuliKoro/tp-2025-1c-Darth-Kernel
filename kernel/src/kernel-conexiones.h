#ifndef KERNEL_CONEXIONES_H_
#define KERNEL_CONEXIONES_H_


#include <utils/handshake.h>
#include <utils/sockets.h>
#include <utils/configs.h>
#include <utils/logger.h>

//Conexion con CPU

/**
* @brief Crea la conexion con CPU (kernel es server aca) y manda handshake.
* @return Devuelve 0
*/
int kernel_conectar_a_cpu(void);



/**
* @brief Crea la conexion con Memoria (kernel es cliente en este caso) y manda handshake.
* @return Devuelve 0
*/
int kernel_conectar_a_memoria(void);


/**
* @brief Crea la conexion con IO (kernel es server) y manda handshake.
* @return Devuelve 0
*/
int kernel_conectar_a_io(void);



#endif