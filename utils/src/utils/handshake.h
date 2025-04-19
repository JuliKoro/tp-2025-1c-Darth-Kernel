#ifndef UTILS_HANDSHAKE_H_
#define UTILS_HANDSHAKE_H_

#include "handshake.h"
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <commons/log.h>


/**
* @brief Tipos de modulo para handshake, arranca en 1
*/
typedef enum {
    HANDSHAKE_CPU = 1,
    HANDSHAKE_KERNEL,
    HANDSHAKE_MEMORIA,
    HANDSHAKE_IO
} id_modulo_t;

/**
* @brief Asocia valor del modulo al nombre
* @param modulo el enum modulo
* @return Devuelve el nombre del modulo
*/
const char* nombre_modulo(id_modulo_t modulo);


/**
* @brief Envia handshake
* @param socket el socket al cual se envia el mensaje de handshake
* @param modulo el id del modulo que se envia
* @param logger el logger del modulo
* @return 
*/
int enviar_handshake (int socket, id_modulo_t modulo, t_log* logger);


/**
* @brief Recibe handshake
* @param socket el socket desde el cual se recibe el mensae de handshake
* @param modulo_recibido donde se guardara el valor de handshake recibido para luego evaluar
* @param logger el logger del modulo
* @return 
*/
int recibir_handshake (int socket, id_modulo_t* modulo_recibido, t_log* logger);

#endif
