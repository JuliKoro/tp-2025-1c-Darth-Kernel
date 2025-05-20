#include "manejo-io.h"




void* iniciar_receptor_io() {
    //Inicia el servidor de IO, esta funcion nos devuelve el socket en modo escucha
    int socket_io = iniciar_servidor_io();
    pthread_t thread_receptor_io;

    while(1) {
        int conexion = esperar_cliente(socket_io);
        pthread_create(&thread_receptor_io, NULL, guardar_io, (void*)(intptr_t)conexion);
        pthread_detach(thread_receptor_io);
    }
}

void agregar_io_a_lista(t_io* io) {
    //Agrega el io a la lista de IO
    pthread_mutex_lock(&mutex_io);
    list_add(lista_io, io);
    pthread_mutex_unlock(&mutex_io);
}

void mostrar_lista_io() {
    pthread_mutex_lock(&mutex_io);
    for(int i = 0; i < list_size(lista_io); i++) {
        t_io* io = list_get(lista_io, i);
        log_info(logger_kernel, "[IO] Nombre del IO: %s (socket: %d)", io->nombre_io, io->socket_io);
    }
    pthread_mutex_unlock(&mutex_io);
}

void* guardar_io(void* socket_ptr) {
    int socket_io = (int)(intptr_t)socket_ptr;

    //Recibo el nombre del io
    char* nombre_io;
    if(recibir_handshake_io(socket_io, &nombre_io) == -1) {
        log_error(logger_kernel, "[HANDSHAKE] Error en recepcion de handshake. Cierro conexion.");
        return NULL;
    }

    //Chequeo que lo recibido sea un string
    if(nombre_io == NULL || nombre_io[0] == '\0') {
        log_error(logger_kernel, "[HANDSHAKE] Error en recepcion de handshake. Cierro conexion.");
        return NULL;
    }

    log_info(logger_kernel, "[HANDSHAKE] Handshake recibido correctamente. Conexion establecida con IO");
    log_info(logger_kernel, "[IO] Nombre del IO recibido: %s", nombre_io);
    
    t_io* io = malloc(sizeof(t_io));
    io->nombre_io = nombre_io;
    io->socket_io = socket_io;
    agregar_io_a_lista(io);
    log_info(logger_kernel, "[IO] IO agregado a la lista");

    enviar_mensaje("IO agregado a la lista", socket_io);

    mostrar_lista_io();

    return NULL;
}

