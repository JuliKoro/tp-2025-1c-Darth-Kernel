#ifndef UTILS_SOCKETS_H_
#define UTILS_SOCKETS_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <utils/logger.h>
#include <errno.h>

#define IP "127.0.0.1"
#define PUERTO_KERNEL_CPU "4444"
#define PUERTO_KERNEL_IO "5555" 
#define PUERTO_MEMORIA_KERNEL "6666" 
#define PUERTO_MEMORIA_CPU "7777" 


/**
* @brief Crea un socket para el servidor. Lo pone en modo escucha
* @param puerto el puerto donde va a escuchar el socket del servidor
* @return Devuelve el descriptor del socket
*/
int iniciar_servidor(char* puerto, t_log* logger);


/**
* @brief Obtiene informacion de la ip, crea un socket, lo conecta con el 
* servidor
* @param ip la ip del server
* @param puerto puerto donde escucha el servidor
* @return Devuelve el descriptor del socket conectado, o valor negativo
* si hay error.
*/
int crear_conexion(char* ip, char* puerto, t_log* logger);


/**
* @brief Recibe el socket del servidor y el logger del modulo
* @param socket_servidor el socket del servidor que va a aceptar conexiones
* @param logger el logger del modulo
* @return devuelve un socket conectado al socket del servidor
*/
int esperar_cliente(int socket_servidor, t_log* logger);


/**
* @brief Envia un string al socket que se pasa por parametro
* @param mensaje el mensaje a enviar
* @param socket el socket de destino
* @param logger el logger del modulo que envia el mensaje
* @return Nada 
*/
void enviar_mensaje (char* mensaje, int socket, t_log* logger);


/**
* @brief Recibe el mensaje desde el socket
* @param socket el socket desde donde recibira el mensaje
* @param logger el logger del modulo
* @return devuelve el mensaje recibido char*
*/
char* recibir_mensaje(int socket, t_log* logger);



#endif
