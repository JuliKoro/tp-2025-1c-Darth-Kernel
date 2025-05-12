#ifndef MEMORIA_CONEXIONES_H_
#define MEMORIA_CONEXIONES_H_

#include <utils/sockets.h>

#include "memoria-configs.h"

/**
* @brief Inicia el servidor de memoria
* @return Devuelve socket del cliente para comunicarse con memoria
*/
int iniciar_servidor_memoria();

#endif