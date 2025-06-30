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

bool comparar_socket_io(void* elemento, void* socket_a_comparar) {
    int socket = (int)(intptr_t)socket_a_comparar;
    t_instancia_io* instancia_io = (t_instancia_io*)elemento;
    return instancia_io->socket_io == socket;
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

void disminuir_instancias_disponibles(char* nombre_io) {
    pthread_mutex_lock(&mutex_io);
    t_io* io = list_find_con_param(lista_io, comparar_nombre_io, nombre_io);
    if(io != NULL) {
        io->instancias_disponibles--;
        log_info(logger_kernel, "[IO] Instancias disponibles de %s disminuidas a %d", io->nombre_io, io->instancias_disponibles);
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

t_io* buscar_io_por_socket(int socket_io){
    pthread_mutex_lock(&mutex_io);
    t_io* io = list_find_con_param(lista_io, comparar_socket_io, (void*)(intptr_t)socket_io);
    pthread_mutex_unlock(&mutex_io);
    return io;
}

t_blocked_io* buscar_proceso_en_blocked_io(int socket_io) {
    t_io* io = buscar_io_por_socket(socket_io);
    if(io == NULL) {
        log_error(logger_kernel, "[IO] No se encontró el IO con socket %d", socket_io);
        return NULL;
    }

    pthread_mutex_lock(&mutex_lista_blocked_io);

    for(int i = 0; i < list_size(lista_blocked_io); i++) {
        t_blocked_io* pcb_blocked = list_get(lista_blocked_io, i);
        if(strcmp(pcb_blocked->nombre_io, io->nombre_io) == 0) {
            pthread_mutex_unlock(&mutex_lista_blocked_io);
            return pcb_blocked;
        }
    }

    pthread_mutex_unlock(&mutex_lista_blocked_io);
    
    return NULL;
}



void* atender_io(void* instancia_io){
    t_instancia_io* instancia_io_a_manejar = (t_instancia_io*)instancia_io;
    int socket_io = instancia_io_a_manejar->socket_io;

    

    while(1){
        char* mensaje = recibir_mensaje(socket_io);

        if (mensaje == NULL){ //Si es NULL es porque se cerro la conexion
            log_info(logger_kernel, "[IO] Instancia de IO %d desconectada", socket_io);

            pthread_mutex_lock(&mutex_io); //Critica
            
            //Si habia un proceso usando la instancia que se desconecto, lo muevo a EXIT
            if(instancia_io_a_manejar->pid_actual != -1){
                log_info(logger_kernel, "[IO] PID %d estaba usando la instancia desconectada. Pasa a EXIT", instancia_io_a_manejar->pid_actual);
                mover_pcb_a_exit_desde_executing(instancia_io_a_manejar->pid_actual);
            }

            //Elimino la instancia de la lista
            eliminar_instancia_io(socket_io);

            //Cierro el socket de esa instancia
            close(socket_io);
            pthread_mutex_unlock(&mutex_io);// Termina critica
            free(instancia_io_a_manejar);
            break;
        }

        if (strcmp(mensaje, "IO finalizada") == 0){
            log_info(logger_kernel, "[IO] IO finalizada");
            pthread_mutex_lock(&mutex_io);

            int pid = instancia_io_a_manejar->pid_actual;
            instancia_io_a_manejar->pid_actual = -1;

            t_io* io = buscar_io_por_socket(socket_io); 
            aumentar_instancias_disponibles(io->nombre_io);

            mover_pcb_a_ready_desde_blocked(pid);
            log_info(logger_kernel, "[IO] PID (%d) finalizo IO y pasa a READY", pid);

            //Me fijo si hay un proceso esperando usar esta IO que se libero, si la hay
            //tengo que mandar la solicitud a la IO.
            t_blocked_io* pcb_en_blocked = buscar_proceso_en_blocked_io(socket_io);
            
            if(pcb_en_blocked != NULL){
                log_info(logger_kernel, "[IO] PID (%d) estaba bloqueado esperando a IO %s", pcb_en_blocked->pid, pcb_en_blocked->nombre_io);
                log_info(logger_kernel, "[IO] PID (%d) encontrado en blocked_io. Proceso sigue en blocked general, pero envio solicitud a IO", pcb_en_blocked->pid);
                //Envio solicitud a IO
                instancia_io_a_manejar->pid_actual = pcb_en_blocked->pid;
                disminuir_instancias_disponibles(pcb_en_blocked->nombre_io);

                  t_solicitud_io solicitud_io = {
                    .pid = pcb_en_blocked->pid,
                    .tiempo = pcb_en_blocked->tiempo_io
                };


                t_buffer* buffer_io = serializar_solicitud_io(&solicitud_io);
                t_paquete* paquete_io = empaquetar_buffer(PAQUETE_SOLICITUD_IO, buffer_io);

                if(enviar_paquete(socket_io, paquete_io) == -1){
                    log_error(logger_kernel, "[IO] Error al enviar solicitud de IO a IO");
                    return NULL;
                }

                sacar_pcb_de_blocked_io(pcb_en_blocked); //Implementar

                liberar_paquete(paquete_io);
                free(pcb_en_blocked->nombre_io);
                free(pcb_en_blocked);


            }
            

            
        }

        free(mensaje);
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
        log_info(logger_kernel, "[IO] IO no encontrada en la lista, la agrego");\
        //Reservo memoria para la IO
        t_io* io_nueva = malloc(sizeof(t_io));
        if(io_nueva == NULL) {
            log_error(logger_kernel, "[IO] Error al reservar memoria para la IO");
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }
        log_info(logger_kernel, "[IO] Memoria para IO nueva reservada");

        //Copio el nombre del IO
        io_nueva->nombre_io = strdup(nombre_io);
        log_info(logger_kernel, "[IO] Nombre del IO copiado");
        free(nombre_io);

        //Creo la lista de instancias para este IO
        io_nueva->instancias_io = list_create(); 
        if(io_nueva->instancias_io == NULL) {
            log_error(logger_kernel, "[IO] Error al reservar memoria para la lista de instancias");
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }
        log_info(logger_kernel, "[IO] Lista de instancias creada");

        //Creo la instancia de IO
        t_instancia_io* instancia_io = malloc(sizeof(t_instancia_io)); 
        if(instancia_io == NULL) {
            log_error(logger_kernel, "[IO] Error al reservar memoria para la instancia de IO");
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }
        log_info(logger_kernel, "[IO] Instancia de IO creada");

        //Agrego la instancia en la lista de instancias del IO
        instancia_io->socket_io = socket_io;
        instancia_io->pid_actual = -1;
        //Agrego la instancia en la lista de instancias del IO
        list_add(io_nueva->instancias_io, instancia_io); 
        io_nueva->instancias_disponibles = 1;
        log_info(logger_kernel, "[IO] Instancias disponibles de IO aumentadas a 1");
        agregar_io_a_lista(io_nueva);
        log_info(logger_kernel, "[IO] IO %s agregado a la lista", io_nueva->nombre_io);

        //Despues de agregar la IO a la lista, creo el hilo de atencion. Este hilo mantiene viva la conexion con el modulo 
        //y maneja posibles desconexiones.

         pthread_t hilo_atencion_io;

        if(pthread_create(&hilo_atencion_io, NULL, atender_io, (void*)instancia_io) == 0) {
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

        if(pthread_create(&hilo_atencion_io, NULL, atender_io, (void*)instancia_io) == 0) {
            pthread_detach(hilo_atencion_io);
        }
    }
    //pthread_mutex_unlock(&mutex_io);
    
    enviar_mensaje("IO agregado a la lista", socket_io);

   
   


    //mostrar_lista_io();

    return NULL;
}

