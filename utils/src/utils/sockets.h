#ifndef UTILS_SOCKETS_H_
#define UTILS_SOCKETS_H_

// Includes del sistema
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

// Includes de commons
#include <commons/log.h>
#include <commons/config.h>

// Includes locales
#include <utils/logger.h>

#define IPLOCAL "127.0.0.1"

extern t_log* logger_sockets;

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
* @brief Crea un socket para el servidor. Lo pone en modo escucha
* @param puerto el puerto donde va a escuchar el socket del servidor
* @return Devuelve el descriptor del socket
*/
int iniciar_servidor(const char* puerto);

/**
* @brief Obtiene informacion de la ip, crea un socket, lo conecta con el 
* servidor
* @param ip la ip del server
* @param puerto puerto donde escucha el servidor
* @return Devuelve el descriptor del socket conectado, o valor negativo
* si hay error.
*/
int crear_conexion(const char* ip, const char* puerto);

/**
* @brief Recibe el socket del servidor y el logger del modulo
* @param socket_servidor el socket del servidor que va a aceptar conexiones
* @param logger el logger del modulo
* @return devuelve un socket conectado al socket del servidor
*/
int esperar_cliente(int socket_servidor);

/**
* @brief Envia un string al socket que se pasa por parametro
* @param mensaje el mensaje a enviar
* @param socket el socket de destino
* @param logger el logger del modulo que envia el mensaje
* @return Nada 
*/
void enviar_mensaje(char* mensaje, int socket);

/**
* @brief Recibe el mensaje desde el socket
* @param socket el socket desde donde recibira el mensaje
* @param logger el logger del modulo
* @return devuelve el mensaje recibido char*
*/
char* recibir_mensaje(int socket);

/**
* @brief Convierte un int a string. Basicamente porque cargamos los puertos como int, pero para crear el socket
* necesitamos un string.
* @param numero el numero a convertir
* @return devuelve el numero convertido a string
*/
const char* int_a_string(int numero);


/**
* @brief Envia handshake
* @param socket el socket al cual se envia el mensaje de handshake
* @param modulo el id del modulo que se envia
* @param logger el logger del modulo
* @return 
*/
int enviar_handshake(int socket, id_modulo_t modulo);

/**
* @brief Recibe handshake
* @param socket el socket desde el cual se recibe el mensae de handshake
* @param modulo_recibido donde se guardara el valor de handshake recibido para luego evaluar
* @param logger el logger del modulo
* @return 
*/
int recibir_handshake(int socket, id_modulo_t* modulo_recibido);


#endif