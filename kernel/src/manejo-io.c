#include "manejo-io.h"




void* iniciar_receptor_io() {
    //Inicia el servidor de IO, esta funcion nos devuelve el socket en modo escucha
    int socket_io = iniciar_servidor_io();
    pthread_t thread_receptor_io;

    while(1) {
        int conexion = esperar_cliente(socket_io);
        pthread_create(&thread_receptor_io, NULL, guardar_io, (void*)(intptr_t)conexion);
        pthread_join(thread_receptor_io, NULL); //Probando con join
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
    list_add(lista_io, io);
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

void aumentar_instancias_disponibles_unsafe(char* nombre_io) {
    t_io* io = list_find_con_param(lista_io, comparar_nombre_io, nombre_io);
    if(io != NULL) {
        io->instancias_disponibles++;
        log_info(logger_kernel, "[IO] Instancias disponibles de %s aumentadas a %d", io->nombre_io, io->instancias_disponibles);
    } else{
        log_error(logger_kernel, "[IO] IO no encontrada");
    }
}

void aumentar_instancias_disponibles(char* nombre_io) {
    pthread_mutex_lock(&mutex_io);
    aumentar_instancias_disponibles_unsafe(nombre_io);
    pthread_mutex_unlock(&mutex_io);
}

void disminuir_instancias_disponibles_unsafe(char* nombre_io) {
    t_io* io = list_find_con_param(lista_io, comparar_nombre_io, nombre_io);
    if(io != NULL) {
        io->instancias_disponibles--;
        log_info(logger_kernel, "[IO] Instancias disponibles de %s disminuidas a %d", io->nombre_io, io->instancias_disponibles);
    } else{
        log_error(logger_kernel, "[IO] IO no encontrada");
    }
}

void disminuir_instancias_disponibles(char* nombre_io) {
    pthread_mutex_lock(&mutex_io);
    disminuir_instancias_disponibles_unsafe(nombre_io);
    pthread_mutex_unlock(&mutex_io);
}

void eliminar_instancia_io(int socket_io){

    for(int i = 0; i < list_size(lista_io); i++) {
        t_io* io = list_get(lista_io, i);
        for(int j = 0; j < list_size(io->instancias_io); j++) {
            t_instancia_io* instancia_io = list_get(io->instancias_io, j);
            if(instancia_io->socket_io == socket_io) {
                list_remove(io->instancias_io, j);
                log_debug(logger_kernel, "[IO Management] Liberando memoria para instancia de IO (Socket: %d). Dirección: %p", socket_io, (void*)instancia_io); // DEBUG_LOG
                free(instancia_io);
                if(io->instancias_disponibles > 0) {
                    io->instancias_disponibles--;
                    log_info(logger_kernel, "[IO] Instancia de IO eliminada. Instancias disponibles de %s: %d", io->nombre_io, io->instancias_disponibles);
                } else {
                    log_info(logger_kernel, "[IO] Instancia de IO eliminada. Instancias disponibles de %s: %d", io->nombre_io, io->instancias_disponibles);
                }
                
                if(list_is_empty(io->instancias_io)) {

                    list_remove(lista_io, i);
                    free(io->nombre_io);
                    list_destroy(io->instancias_io);
                    log_debug(logger_kernel, "[IO Management] Liberando memoria para IO '%s' (última instancia eliminada). Dirección: %p", io->nombre_io, (void*)io); // DEBUG_LOG
                    free(io);
                    log_info(logger_kernel, "[IO] IO eliminada de la lista");
                }
                return;
            }
        }
    }
    
}

void atender_siguiente_proceso_en_cola_io(t_instancia_io* instancia_liberada){
    // Esta función asume que el mutex_io ya está tomado.
    t_blocked_io* pcb_en_espera = buscar_proceso_en_blocked_io(instancia_liberada->socket_io);

    if(pcb_en_espera != NULL){
        t_io* io_del_proceso = buscar_io_por_socket_unsafe(instancia_liberada->socket_io);
        if(io_del_proceso == NULL){
            log_error(logger_kernel, "Inconsistencia grave: No se pudo encontrar el IO para el socket %d", instancia_liberada->socket_io);
            return;
        }

        log_info(logger_kernel, "[PLANIFICADOR IO] Se encontró al PID (%d) esperando por %s. Se procederá a atenderlo.", pcb_en_espera->pid, pcb_en_espera->nombre_io);
        
        instancia_liberada->pid_actual = pcb_en_espera->pid;
        disminuir_instancias_disponibles_unsafe(io_del_proceso->nombre_io);
        
        t_solicitud_io solicitud_io = { .pid = pcb_en_espera->pid, .tiempo = pcb_en_espera->tiempo_io };
        t_buffer* buffer_io = serializar_solicitud_io(&solicitud_io);
        t_paquete* paquete_io = empaquetar_buffer(PAQUETE_SOLICITUD_IO, buffer_io);

        if(enviar_paquete(instancia_liberada->socket_io, paquete_io) == -1){
            log_error(logger_kernel, "[PLANIFICADOR IO] Error al enviar la solicitud de IO para el PID (%d).", pcb_en_espera->pid);
            instancia_liberada->pid_actual = -1;
            aumentar_instancias_disponibles_unsafe(io_del_proceso->nombre_io);
            mover_blockedio_a_exit(pcb_en_espera->pid);
        } else {
            log_info(logger_kernel, "[PLANIFICADOR IO] Solicitud de IO para PID (%d) enviada correctamente.", pcb_en_espera->pid);
             // Guardamos el PID antes de liberar la memoria para usarlo en el log.
            u_int32_t pid_atendido = pcb_en_espera->pid;

            //Limpiamos la estructura de la solicitud que ya no está en espera.
            sacar_de_blockedio(pcb_en_espera);
            free(pcb_en_espera->nombre_io);
            free(pcb_en_espera);
        
        log_info(logger_kernel, "[PLANIFICADOR IO] Limpieza de solicitud para PID (%d) finalizada.", pid_atendido);
        }
        
        
    }
}

void* atender_io(void* instancia_io){
    t_instancia_io* instancia_io_a_manejar = (t_instancia_io*)instancia_io;
    int socket_io = instancia_io_a_manejar->socket_io;

    

    while(1){
        char* mensaje = recibir_mensaje(socket_io);

        if (mensaje == NULL){ //Si es NULL es porque se cerro la conexion
            log_info(logger_kernel, "[IO] Instancia de IO (socket %d) desconectada", socket_io);

            pthread_mutex_lock(&mutex_io); //Critica
            
            //Si habia un proceso usando la instancia que se desconecto, lo muevo a EXIT
            if(instancia_io_a_manejar->pid_actual != -1){
                log_info(logger_kernel, "[IO] PID %d estaba usando la instancia desconectada. Pasa a EXIT", instancia_io_a_manejar->pid_actual);
                mover_blocked_a_exit(instancia_io_a_manejar->pid_actual);
            }

            //Elimino la instancia de la lista
            eliminar_instancia_io(socket_io);

            //Cierro el socket de esa instancia
            close(socket_io);
            pthread_mutex_unlock(&mutex_io);// Termina critica
            break;
        }

        if (strcmp(mensaje, "IO finalizada") == 0){
            // log_info(logger_kernel, "[IO] IO finalizada para el proceso con PID (%d)", ((t_instancia_io*)instancia_io)->pid_actual);
            pthread_mutex_lock(&mutex_io);

            // Obtenemos el PID que terminó
            int pid_que_termino = instancia_io_a_manejar->pid_actual;
            instancia_io_a_manejar->pid_actual = -1;

            t_io* io = buscar_io_por_socket_unsafe(socket_io); 
            aumentar_instancias_disponibles_unsafe(io->nombre_io);
            
            //Chequeo si el proceso esta en blocked, o susp blocked para ver si moverlo a ready o susp ready.
            estado_pcb estado_pcb;
            //primero veo si esta en blocked
            pthread_mutex_lock(&mutex_lista_blocked);
            for(int i = 0; i < list_size(lista_blocked); i++) {
                t_pcb* pcb_temp = list_get(lista_blocked, i);
                if(pcb_temp->pid == pid_que_termino) {
                    estado_pcb = pcb_temp->estado;
                }
            }
            pthread_mutex_unlock(&mutex_lista_blocked);

            //Luego veo si esta en susp blocked
            pthread_mutex_lock(&mutex_lista_suspblocked);
            for(int i = 0; i < list_size(lista_suspblocked); i++) {
                t_pcb* pcb_temp = list_get(lista_suspblocked, i);
                if(pcb_temp->pid == pid_que_termino) {
                    estado_pcb = pcb_temp->estado;
                }
            }
            pthread_mutex_unlock(&mutex_lista_suspblocked);


            if(estado_pcb == BLOCKED){
                mover_blocked_a_ready(pid_que_termino);
            } else {
                mover_blocked_a_suspblocked(pid_que_termino);
            }

            log_info(logger_kernel, "## <%d> finalizó IO y pasa a %s", pid_que_termino, estado_pcb == BLOCKED ? "READY" : "SUSP_READY");


            atender_siguiente_proceso_en_cola_io(instancia_io_a_manejar);

            pthread_mutex_unlock(&mutex_io);
        }

        free(mensaje);
    }

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

        //Copio el nombre del IO
        io_nueva->nombre_io = strdup(nombre_io);
        free(nombre_io);

        //Creo la lista de instancias para este IO
        io_nueva->instancias_io = list_create(); 
        if(io_nueva->instancias_io == NULL) {
            log_error(logger_kernel, "[IO] Error al reservar memoria para la lista de instancias");
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }

        //Creo la instancia de IO
        t_instancia_io* instancia_io = malloc(sizeof(t_instancia_io)); 
        if(instancia_io == NULL) {
            log_error(logger_kernel, "[IO] Error al reservar memoria para la instancia de IO");
            pthread_mutex_unlock(&mutex_io);
            return NULL;
        }

        //Agrego la instancia en la lista de instancias del IO
        instancia_io->socket_io = socket_io;
        instancia_io->pid_actual = -1;
        //Agrego la instancia en la lista de instancias del IO
        list_add(io_nueva->instancias_io, instancia_io); 
        io_nueva->instancias_disponibles = 1;
        log_info(logger_kernel, "[IO] Instancias disponibles de IO aumentadas a 1");
        log_debug(logger_kernel, "[IO Management] Creada nueva IO '%s' con 1 instancia. Dirección IO: %p, Dirección Instancia: %p", io_nueva->nombre_io, (void*)io_nueva, (void*)instancia_io); // DEBUG_LOG
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
        log_debug(logger_kernel, "[IO Management] Creada nueva instancia para IO '%s'. Dirección Instancia: %p", io->nombre_io, (void*)instancia_io); // DEBUG_LOG

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

