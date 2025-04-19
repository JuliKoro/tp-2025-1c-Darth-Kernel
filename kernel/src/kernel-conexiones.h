#ifndef KERNEL_CONEXIONES_H_
#define KERNEL_CONEXIONES_H_

#include <utils/handshake.h>
#include <utils/sockets.h>


//Conexion con CPU

int kernel_conectar_a_cpu(void);
int kernel_conectar_a_memoria(void);
int kernel_conectar_a_io();



#endif