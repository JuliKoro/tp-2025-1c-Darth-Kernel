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

bool comparar_nombre_io(void* elemento, void* nombre_a_comparar) {
    t_io* io = (t_io*)elemento;
    char* nombre_io = (char*)nombre_a_comparar;
    return strcmp(io->nombre_io, nombre_io) == 0;
}




void agregar_io_a_lista(t_io* io) {
    //Agrega el io a la lista de IO
    pthread_mutex_lock(&mutex_io);
    log_info(logger_kernel, "[DEBUG] Agregando IO a la lista");
    list_add(lista_io, io);
    log_info(logger_kernel, "[DEBUG] IO agregada a la lista");
    pthread_mutex_unlock(&mutex_io);
}

void mostrar_lista_io() {
    pthread_mutex_lock(&mutex_io);
    log_info(logger_kernel, "[IO] Lista de IO:");
    for(int i = 0; i < list_size(lista_io); i++) {
        t_io* io = list_get(lista_io, i);
        log_info(logger_kernel, "[IO] Nombre del IO: %s (cantidad de instancias: %d)", io->nombre_io, io->instancias_disponibles);
        for(int j = 0; j < list_size(io->instancias_io); j++) {
            t_instancia_io* instancia_io = list_get(io->instancias_io, j);
            log_info(logger_kernel, "[IO] Instancia %d: socket: %d, pid: %d", j, instancia_io->socket_io, instancia_io->pid_actual);
        }
    }
    pthread_mutex_unlock(&mutex_io);
}

void aumentar_instancias_disponibles(char* nombre_io) {
    pthread_mutex_lock(&mutex_io);
    t_io* io = list_find_con_param(lista_io, comparar_nombre_io, nombre_io);
    if(io != NULL) {
        io->instancias_disponibles++;
        log_info(logger_kernel, "[IO] Instancias disponibles de %s aumentadas a %d", io->nombre_io, io->instancias_disponibles);
    } else{
        log_error(logger_kernel, "[IO] IO no encontrada");
    }

    pthread_mutex_unlock(&mutex_io);
}

void eliminar_instancia_io(int socket_io){
    pthread_mutex_lock(&mutex_io);

    for(int i = 0; i < list_size(lista_io); i++) {
        t_io* io = list_get(lista_io, i);
        for(int j = 0; j < list_size(io->instancias_io); j++) {
            t_instancia_io* instancia_io = list_get(io->instancias_io, j);
            if(instancia_io->socket_io == socket_io) {
                list_remove(io->instancias_io, j);
                free(instancia_io);
                io->instancias_disponibles--;
                log_info(logger_kernel, "[IO] Instancia de IO eliminada. Instancias disponibles de %s: %d", io->nombre_io, io->instancias_disponibles);
                
                if(list_is_empty(io->instancias_io)) {

                    list_remove(lista_io, i);
                    free(io);
                    log_info(logger_kernel, "[IO] IO eliminada de la lista");
                }
                break;
            }
        }
    }
    
    pthread_mutex_unlock(&mutex_io);
}

void* atender_io(void* socket_ptr){
    int socket_io = (int)(intptr_t)socket_ptr;

    char buffer[1];
    ssize_t bytes_recibidos;


    while(1){
        bytes_recibidos = recv(socket_io, buffer, sizeof(buffer), 0);
        if(bytes_recibidos == 0) {
            log_warning(logger_kernel, "[IO] Conexion cerrada en el socket %d", socket_io);
            break;
        } else if(bytes_recibidos < 0) {
            log_error(logger_kernel, "[IO] Error al recibir datos de IO. Cerrando conexion.");
            break;
        }
    }

    close(socket_io);
    eliminar_instancia_io(socket_io);
    return NULL;
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
    
    //Me fijo si la IO ya existe, si existe aumento instancias. Si no existe la agrego a la lista.
    //pthread_mutex_lock(&mutex_io);
    
    log_info(logger_kernel, "[DEBUG] lista_io %s, tamaño: %d", 
         (lista_io != NULL) ? "inicializada" : "NO inicializada",
         (lista_io != NULL) ? list_size(lista_io) : -1);
    
    t_io* io = list_find_con_param(lista_io, comparar_nombre_io, nombre_io);

    if(io == NULL) { //No existe en la lista, la agrego
        log_info(logger_kernel, "[IO] IO no encontrada en la lista, la agrego");
        t_io* io_nueva = malloc(sizeof(t_io));
        if(io_nueva == NULL) {
            log_error(logger_kernel, "[IO] Error al reservar memoria para la IO");
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }
        log_info(logger_kernel, "[IO] IO nueva reservada");


        if(pthread_mutex_init(&io_nueva->mutex_cola_blocked_io,  NULL) != 0) {
            log_error(logger_kernel, "[IO] Error al inicializar mutex de cola blocked");
            free(io_nueva);
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }
        
        log_info(logger_kernel, "[IO] Mutex de cola blocked inicializado");

        io_nueva->nombre_io = strdup(nombre_io);
        log_info(logger_kernel, "[IO] Nombre del IO copiado");
        free(nombre_io);

        io_nueva->instancias_io = list_create(); //Creo la lista de instancias para este IO
        if(io_nueva->instancias_io == NULL) {
            log_error(logger_kernel, "[IO] Error al reservar memoria para la lista de instancias");
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }
        log_info(logger_kernel, "[IO] Lista de instancias creada");
        io_nueva->cola_blocked_io = queue_create(); //Creo la cola de procesos bloqueados para este IO
        if(io_nueva->cola_blocked_io == NULL) {
            log_error(logger_kernel, "[IO] Error al reservar memoria para la cola de procesos bloqueados");
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }
        log_info(logger_kernel, "[IO] Cola de procesos bloqueados creada");
        t_instancia_io* instancia_io = malloc(sizeof(t_instancia_io)); //Creo una instancia de este IO
        if(instancia_io == NULL) {
            log_error(logger_kernel, "[IO] Error al reservar memoria para la instancia de IO");
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }
        log_info(logger_kernel, "[IO] Instancia de IO creada");
        instancia_io->socket_io = socket_io;
        instancia_io->pid_actual = -1;
        list_add(io_nueva->instancias_io, instancia_io); //Agrego esta instancia en la lista de instancias del IO
        io_nueva->instancias_disponibles = 1;
        log_info(logger_kernel, "[IO] Instancias disponibles de IO aumentadas a 1");
        agregar_io_a_lista(io_nueva);
        log_info(logger_kernel, "[IO] IO %s agregado a la lista", io_nueva->nombre_io);

        //Despues de agregar la IO a la lista, creo el hilo de atencion

         pthread_t hilo_atencion_io;

        if(pthread_create(&hilo_atencion_io, NULL, atender_io, (void*)(intptr_t)socket_io) == 0) {
            pthread_detach(hilo_atencion_io);
        } else {
            log_error(logger_kernel, "[IO] Error al crear hilo de atencion de IO");
        }

        
    } else {
        t_instancia_io* instancia_io = malloc(sizeof(t_instancia_io)); //Creo una instancia de este IO
        if(instancia_io == NULL) {
            log_error(logger_kernel, "[IO] Error al reservar memoria para la instancia de IO");
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }

        instancia_io->socket_io = socket_io;
        instancia_io->pid_actual = -1;
        list_add(io->instancias_io, instancia_io); //Agrego esta instancia en la lista de instancias del IO
        io->instancias_disponibles++;
        log_info(logger_kernel, "[IO] Instancias disponibles de %s aumentadas a %d", io->nombre_io, io->instancias_disponibles);

        //Despues de agregar la instancia a la lista, creo el hilo de atencion

        pthread_t hilo_atencion_io;

        if(pthread_create(&hilo_atencion_io, NULL, atender_io, (void*)(intptr_t)socket_io) == 0) {
            pthread_detach(hilo_atencion_io);
        }
    }
    //pthread_mutex_unlock(&mutex_io);
    
    enviar_mensaje("IO agregado a la lista", socket_io);

   
   


    //mostrar_lista_io();

    return NULL;
}

