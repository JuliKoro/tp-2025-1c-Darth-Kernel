#include "sockets.h"
#include <commons/log.h>



int iniciar_servidor(char* puerto, t_log* logger){
    int socket_servidor;
     
    struct addrinfo hints, *server_info;

    //Seteo hints en 0
    memset(&hints,0, sizeof(hints));
    hints.ai_family = AF_INET; //IPV4
    hints.ai_socktype = SOCK_STREAM; //TCP
    hints.ai_flags = AI_PASSIVE; 

    //Obtengo info del puerto e ip local, manejo errores
    int err = getaddrinfo(NULL, puerto, &hints, &server_info);
    if(err !=0){
        log_error(logger, "Fallo getaddrinfo - %s", gai_strerror(err));
    }

    //Creo el socket, chequeo errores
    socket_servidor = socket (server_info->ai_family,
                              server_info->ai_socktype,
                              server_info->ai_protocol);

    if (socket_servidor == -1){
        log_error(logger, "Error al crear el socket: %s", strerror(errno));
        freeaddrinfo(server_info);
        return -1;
    }
    
    //Para que pueda reusar puertos, mencionado en tp0
    setsockopt(socket_servidor, SOL_SOCKET,SO_REUSEPORT,&(int){1}, sizeof(int));

    //Bindeo el socket al puerto
	if(bind(socket_servidor,server_info->ai_addr,server_info->ai_addrlen) == -1) {
        log_error(logger, "Error en el bind: %s", strerror(errno));
        close(socket_servidor);
        freeaddrinfo(server_info);
        return -1;
    }

    //Escucho conexiones entrantes
	if(listen(socket_servidor, SOMAXCONN) == -1) {
        log_error(logger, "Error en listen: %s", strerror(errno));
        close(socket_servidor);
        freeaddrinfo(server_info);
        return -1;
    }

    //Libero memoria
    freeaddrinfo(server_info);
    
    //Loggeo y devuelvo socket
    log_info(logger, "Servidor escuchando en el puerto %s !", puerto);
    return socket_servidor;
}

int crear_conexion (char* ip, char* puerto, t_log* logger){
    
    struct addrinfo hints, *server_info;

    //Seteo hints en 0
    memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; //IPV4
	hints.ai_socktype = SOCK_STREAM; //TCP
    hints.ai_flags = AI_PASSIVE;

    //Obtengo informacion de la IP, puerto y chequeo errores
    int err = getaddrinfo(ip, puerto, &hints, &server_info);
    if(err !=0){
        log_error(logger, "Fallo getaddrinfo al conectar con %s:%s - %s", ip, puerto, gai_strerror(err));
    }
        

    //Creo el socket
    int socket_cliente = socket (server_info->ai_family, 
                                 server_info->ai_socktype, 
                                 server_info->ai_protocol);

    //Chequeo por errores en la creacion
    if(socket_cliente == -1) {
        log_error(logger,"Error al crear socket %s ", strerror(errno));
        freeaddrinfo(server_info);
        return -1;
    }

    //Se conecta al socket con el server, el cual debe estar escuchando. 
    //Chequeo errores              
    if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
        log_error(logger,"Fallo getaddrinfo al conectar con %s:%s - %s", ip, puerto, strerror(errno));
        freeaddrinfo(server_info);
        close(socket_cliente);
        return -1;
    }
    
    log_info(logger, "Conectado con exito a %s puerto %s", ip, puerto);

    //Libero memoria 
    freeaddrinfo(server_info);

    //Devuelvo descriptor del socket conectado
    return socket_cliente;
}

int esperar_cliente (int socket_servidor, t_log* logger){


    //Acepta conexion, chequeo errores
    int socket_cliente = accept(socket_servidor, NULL, NULL);
	if(socket_cliente == -1){
        log_error(logger, "Error en el accept(): %s", strerror(errno));
        return -1;
    }

    log_info(logger, "Se conecto un cliente!");

    //Devuelvo socket ya conectado con el servidor
	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket, t_log* logger){
    if(mensaje == NULL || logger ==  NULL){
        return;
    }
    //Tamanio del string +1 para el /0
    int size = strlen(mensaje) + 1;
    //Envio tamanio del string y chequeo errores
    if(send(socket, &size, sizeof(int), 0) == -1){
        log_error(logger, "Error al enviar tamanio del mensaje: %s", strerror(errno));
        return;
    } 
    //Envio mensaje y chequeo errores
    if (send (socket, mensaje, size, 0) == -1){
        log_error(logger, "Error al enviar mensaje: %s", strerror(errno));
        return;
    }
    
}

char* recibir_mensaje(int socket, t_log* logger){
    int size;
    //Recibo tamanio del mensaje a recibir, chequeo errores
    if(recv(socket, &size, sizeof(int), MSG_WAITALL) == -1){
        log_error(logger, "Error al recibir tamanio del mensaje: %s", strerror(errno));
        return NULL;
    }
    //Reservo espacio para recibir el mensaje
    char* buffer = malloc (size);
    //Recibo mensaje, chequeo errores
    if(recv(socket, buffer, size, MSG_WAITALL) == -1){
        log_error(logger, "Error al recibir el mensaje: %s", strerror(errno));
        //Libero memoria reservada para buffer antes de volver
        free(buffer);
        return NULL;
    }
    return buffer;
}
