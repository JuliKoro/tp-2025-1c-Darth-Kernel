#include "planificacion.h"



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Globales

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

u_int32_t grado_multiprogramacion = 0;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Colas Y listas de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_queue* cola_new = NULL;
t_queue* cola_ready = NULL;

t_list* lista_ready_pmcp = NULL;
t_list* lista_exit = NULL;
t_list* lista_blocked = NULL;
t_list* lista_executing = NULL;
t_list* lista_susp_ready = NULL;
t_list* lista_susp_blocked = NULL;
t_list* lista_blocked_io = NULL;

t_list* lista_io = NULL;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones auxiliares a estructuras

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_blocked_io* crear_blocked_io(u_int32_t pid, char* nombre_io, u_int32_t tiempo_io){
    t_blocked_io* blocked_io = malloc(sizeof(t_blocked_io));
    blocked_io->pid = pid;
    blocked_io->nombre_io = nombre_io;
    blocked_io->tiempo_io = tiempo_io;
    return blocked_io;
}

bool comparar_pid(void* elemento, void* pid) {
    t_pcb* pcb = (t_pcb*)elemento;
    u_int32_t pid_pcb = pcb->pid;
    return pid_pcb == *(u_int32_t*)pid;
}


int actualizar_estado_pcb(u_int32_t pid, estado_pcb estado) {
    t_pcb* pcb = list_find_con_param(lista_executing, comparar_pid, &pid);
    pcb->estado = estado;
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                            Semaforos

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pthread_mutex_t mutex_io = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_exit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_executing = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_susp_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_susp_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_blocked_io = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pid_counter = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_grado_multiprogramacion = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_procesos_en_new;
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                            Funciones de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


algoritmos_de_planificacion obtener_algoritmo_de_planificacion(char* algoritmo) {
    if(strcmp(algoritmo, "FIFO") == 0) {
        return FIFO;
    } else if(strcmp(algoritmo, "PMCP") == 0) {
        return PMCP;
    } else if(strcmp(algoritmo, "SJF_SIN_DESALOJO") == 0) {
        return SJF_SIN_DESALOJO;
    } else if(strcmp(algoritmo, "SFJ_CON_DESALOJO") == 0)   {
        return SFJ_CON_DESALOJO;
    }
}

void planificar_proceso_inicial(char* archivo_pseudocodigo, u_int32_t tamanio_proceso) {
    t_pcb* pcb = inicializar_pcb(obtener_pid_actual(), archivo_pseudocodigo, tamanio_proceso);
    pthread_mutex_lock(&mutex_pid_counter);
    pid_counter++;
    pthread_mutex_unlock(&mutex_pid_counter);
    agregar_pcb_a_cola_new(pcb);
    //Loggear ## (<PID>) Se crea el proceso - Estado: NEW
    log_info(logger_kernel, "## (%d) Se crea el proceso inicial - Estado: NEW", pcb->pid);
}

void inicializar_colas_y_sem() {
    cola_new = queue_create();
    cola_ready = queue_create();
    lista_exit = list_create();
    lista_blocked  = list_create();
    lista_executing = list_create();
    lista_susp_ready = list_create();
    lista_susp_blocked = list_create();
    lista_blocked_io = list_create();
    sem_init(&sem_procesos_en_new, 0, 0);
}

int recibir_mensaje_cpu(int socket_cpu) {
    t_paquete* paquete = recibir_paquete(socket_cpu);
    if(paquete == NULL) {
        log_error(logger_kernel, "Error al recibir mensaje (paquete) de CPU");
        return -1;
    }
    
    switch(paquete->codigo_operacion) {
        
        case PAQUETE_SYSCALL:
            log_info(logger_kernel, "Se recibio un pedido de syscall de CPU");
            t_syscall* syscall = deserializar_syscall(paquete->buffer);
            if(syscall == NULL) {
                log_error(logger_kernel, "Error al deserializar syscall");
                return -1;
            }
            //llamo a la funcion de manejo de la syscall en formato string
            manejar_syscall(syscall);
            free(syscall);
            free(paquete);
        default:
            log_error(logger_kernel, "Codigo de operacion invalido recibido de CPU");
            return -1;
    }
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                            Funciones para PIDs

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


u_int32_t obtener_pid_siguiente() {
    pthread_mutex_lock(&mutex_pid_counter);
    pid_counter++;
    pthread_mutex_unlock(&mutex_pid_counter);
    return pid_counter;
}

u_int32_t obtener_pid_actual() {
    pthread_mutex_lock(&mutex_pid_counter);
    u_int32_t pid_actual = pid_counter;
    pthread_mutex_unlock(&mutex_pid_counter);
    return pid_actual;
}



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones para manejo de las colas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void agregar_pcb_a_cola_new(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_cola_new);
    queue_push(cola_new, pcb);
    pthread_mutex_unlock(&mutex_cola_new);
    log_info(logger_kernel, "## (%d) Se crea el proceso - Estado: NEW", pcb->pid);
    sem_post(&sem_procesos_en_new); //Aviso al planificador largo plazo que hay un proceso en new
}


void agregar_pcb_a_cola_ready(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_cola_ready);
    queue_push(cola_ready, pcb);
    pthread_mutex_unlock(&mutex_cola_ready);
}




/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para peekear

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_pcb* peek_cola_new() {
    pthread_mutex_lock(&mutex_cola_new);
    t_pcb* pcb = queue_peek(cola_new);
    pthread_mutex_unlock(&mutex_cola_new);
    return pcb;
}   

t_pcb* peek_cola_ready() {
    pthread_mutex_lock(&mutex_cola_ready);
    t_pcb* pcb = queue_peek(cola_ready);
    pthread_mutex_unlock(&mutex_cola_ready);
    return pcb;
}   

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para obtener pcb de las listas y colas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_pcb* obtener_pcb_de_cola_new() {
    pthread_mutex_lock(&mutex_cola_new);
    t_pcb* pcb = queue_pop(cola_new);
    pthread_mutex_unlock(&mutex_cola_new);
    return pcb;
}

t_pcb* obtener_pcb_de_cola_ready() {
    pthread_mutex_lock(&mutex_cola_ready);
    t_pcb* pcb = queue_pop(cola_ready);
    pthread_mutex_unlock(&mutex_cola_ready);
    return pcb;
}



int mover_pcb_executing_a_blocked_io(u_int32_t pid, char* nombre_io, u_int32_t tiempo_io) {
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_executing);
    
    // Buscar el PCB en la lista
    for(int i = 0; i < list_size(lista_executing); i++) {
        t_pcb* pcb_temp = list_get(lista_executing, i);
        if(pcb_temp->pid == pid) {
            pcb_encontrado = pcb_temp;
            //Tambien lo muevo a la lista de blocked general
            //Actualizo el estado del proceso
            pcb_encontrado->estado = BLOCKED;
            pthread_mutex_lock(&mutex_lista_blocked);
            list_add(lista_blocked, pcb_encontrado);
            pthread_mutex_unlock(&mutex_lista_blocked);

            //Antes de removerlo, lo muevo a blocked_io

            pthread_mutex_lock(&mutex_lista_blocked_io);
            //Tengo que armar un struct de blocked_io
            t_blocked_io* blocked_io = crear_blocked_io(pcb_encontrado->pid, nombre_io, tiempo_io);
            list_add(lista_blocked_io, blocked_io);
            pthread_mutex_unlock(&mutex_lista_blocked_io);

            // Remover el elemento de la cola en la posición i
            list_remove(lista_executing, i);
            break;
        }
    }
    
    pthread_mutex_unlock(&mutex_lista_executing);

    return 0;
}

int sacar_pcb_de_blocked_io(t_blocked_io* io_a_sacar) {
    pthread_mutex_lock(&mutex_lista_blocked_io);
    int indice_a_eliminar = -1;
    for(int i = 0; i < list_size(lista_blocked_io); i++) {
        t_blocked_io* blocked_io_temp = list_get(lista_blocked_io, i);
        if(blocked_io_temp->pid == io_a_sacar->pid && 
           strcmp(blocked_io_temp->nombre_io, io_a_sacar->nombre_io) == 0 &&
           blocked_io_temp->tiempo_io == io_a_sacar->tiempo_io) {
            indice_a_eliminar = i;
            break;
        }
    }
    list_remove(lista_blocked_io, indice_a_eliminar);
    pthread_mutex_unlock(&mutex_lista_blocked_io);
    return 0;
}

int mover_pcb_a_exit_desde_executing(u_int32_t pid) {
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_executing);

    for(int i = 0; i < list_size(lista_executing); i++) {
        t_pcb* pcb_temp = list_get(lista_executing, i);
        if(pcb_temp->pid == pid) {
            pcb_encontrado = pcb_temp;
            //Actualizo el estado del proceso
            pcb_encontrado->estado = EXIT;
            //Antes de removerlo, lo muevo a exit
            list_add(lista_exit, pcb_encontrado);
            list_remove(lista_executing, i);
            break;
        }
    }
    
    pthread_mutex_unlock(&mutex_lista_executing);
    
    return 0;
}

int mover_pcb_a_blocked_desde_executing(u_int32_t pid) {
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_executing);

    for(int i = 0; i < list_size(lista_executing); i++) {
        t_pcb* pcb_temp = list_get(lista_executing, i);
        if(pcb_temp->pid == pid) {
            pcb_encontrado = pcb_temp;
            //Actualizo el estado del proceso
            pcb_encontrado->estado = BLOCKED;
            //Antes de removerlo, lo muevo a blocked
            pthread_mutex_lock(&mutex_lista_blocked);
            list_add(lista_blocked, pcb_encontrado);
            pthread_mutex_unlock(&mutex_lista_blocked);

            list_remove(lista_executing, i);
            break;
        }
    }

    pthread_mutex_unlock(&mutex_lista_executing);

    return 0;
}

int mover_pcb_a_ready_desde_blocked(u_int32_t pid) {
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_blocked);
    for(int i = 0; i < list_size(lista_blocked); i++) {
        t_pcb* pcb_temp = list_get(lista_blocked, i);
        if(pcb_temp->pid == pid) {
            pcb_encontrado = pcb_temp;
            //Actualizo el estado del proceso
            pcb_encontrado->estado = READY;
            //Antes de removerlo, lo muevo a ready
            pthread_mutex_lock(&mutex_cola_ready);
            queue_push(cola_ready, pcb_encontrado);
            pthread_mutex_unlock(&mutex_cola_ready);
            //Despues de actualizar el estado y cambiarlo a lista ready, lo saco de lista blocked
            list_remove(lista_blocked, i);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_blocked);
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para planificacion largo plazo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void mover_procesos_terminados() {
    int elementos_procesados = 0;
    int total_elementos = list_size(lista_executing);
    //Recorro la lista de executing
    while(elementos_procesados < total_elementos) {
        t_pcb* pcb = list_get(lista_executing, elementos_procesados);
        //Si encontre un pcb y su estado es EXIT, lo agrego a la lista de exit
        if(pcb != NULL && pcb->estado == EXIT) {
            list_add(lista_exit, pcb);
            //Lo saco de la lista de executing
            list_remove(lista_executing, elementos_procesados);
        }
        elementos_procesados++;
    }
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para planificacion corto plazo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void aumentar_grado_multiprogramacion() {
    pthread_mutex_lock(&mutex_grado_multiprogramacion);
    grado_multiprogramacion++;
    pthread_mutex_unlock(&mutex_grado_multiprogramacion);
}

void disminuir_grado_multiprogramacion() {
    pthread_mutex_lock(&mutex_grado_multiprogramacion);
    grado_multiprogramacion--;
    pthread_mutex_unlock(&mutex_grado_multiprogramacion);
}

bool comprobar_grado_multiprogramacion_maximo() {
    pthread_mutex_lock(&mutex_grado_multiprogramacion);
    bool valor = grado_multiprogramacion < GRADO_MULTIPROGRAMACION_MAXIMO;
    pthread_mutex_unlock(&mutex_grado_multiprogramacion);
    return valor;
}




/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        SYSCALLS

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones auxiliares para las syscalls

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_io* buscar_io_por_nombre(char* nombre_buscado) {
    pthread_mutex_lock(&mutex_io);

    for(int i = 0; i < list_size(lista_io); i++) {
        t_io* io = list_get(lista_io, i);
        if(strcmp(io->nombre_io, nombre_buscado) == 0) {
            pthread_mutex_unlock(&mutex_io);
            return io;
        }
    }
    pthread_mutex_unlock(&mutex_io);
    return NULL;
}


int manejar_syscall(t_syscall* syscall) {
    //Leo hasta el primer espacio, obtengo syscall
    char* tipo_syscall = strtok(syscall->syscall, " ");

    //Evaluo el tipo de syscall 

    if(strcmp(tipo_syscall, "IO") == 0) {
        //Leo hasta el siguiente espacio, obtengo nombre_io
        char* nombre_io = strtok(NULL, " ");
        //Leo hasta el siguiente espacio, obtengo tiempo_io
        char* tiempo_io = strtok(NULL, " ");
        //Llamo a la funcion io
        if(io(nombre_io, (u_int32_t)atoi(tiempo_io), (u_int32_t)(syscall->pid)) == -1) {
            printf("Error al ejecutar syscall IO\n");
            return -1;
        }
    } else if(strcmp(tipo_syscall, "INIT_PROC") == 0) { 
        //Leo hasta el siguiente espacio, obtengo archivo_pseudocodigo
        char* archivo_pseudocodigo = strtok(NULL, " ");
        //Leo hasta el siguiente espacio, obtengo tamanio_proceso
        char* tamanio_proceso = strtok(NULL, " ");
        //Llamo a la funcion init_proc
        if(init_proc(archivo_pseudocodigo, (u_int32_t)atoi(tamanio_proceso)) == -1) {
            printf("Error al ejecutar syscall INIT_PROC\n");
            return -1;
        }
    } else if(strcmp(tipo_syscall, "EXIT") == 0) {
        //TODO: enviar a exit
    } else if(strcmp(tipo_syscall, "DUMP_MEMORIA") == 0) {
        //TODO: dump_memoria
    } else {
        printf("Syscall no valida\n");
        return -1;
    }
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Syscalls como tal

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


int init_proc(char* archivo_pseudocodigo, u_int32_t tamanio_proceso) {
    t_pcb* pcb = inicializar_pcb(obtener_pid_siguiente(), archivo_pseudocodigo, tamanio_proceso);
    agregar_pcb_a_cola_new(pcb);
    return 0;
} 

int io (char* nombre_io, u_int32_t tiempo_io, u_int32_t pid) {

    pthread_mutex_lock(&mutex_io);

    //Buscar IO en la lista de IO
    t_io* io = buscar_io_por_nombre(nombre_io);

    //Si no existe, proceso va a EXIT
    if(io == NULL) {
        log_info(logger_kernel, "No existe el IO %s", nombre_io);
        actualizar_estado_pcb(pid, EXIT); //Implementar
        log_info(logger_kernel, "## PID (%d) Pasa del estado %s al estado %s", pid, "EXECUTING", "EXIT");
        pthread_mutex_unlock(&mutex_io);
        return -1;
    }

    //Si existe pero no hay instancias, proceso va a BLOCKED IO (tambien va a blocked general, esto se hace en mover_pcb_executing_a_blocked_io)
    if(io->instancias_disponibles == 0) {
        mover_pcb_executing_a_blocked_io(pid, nombre_io, tiempo_io);
        log_info(logger_kernel, "## PID (%d) Pasa del estado %s al estado %s", pid, "EXECUTING", "BLOCKED (por no haber instancias disponibles IO)");
        log_info(logger_kernel, "## PID (%d) - Bloqueado por IO: %s",pid, nombre_io);
        pthread_mutex_unlock(&mutex_io);
        return -1;
    
    } else {//Existe el IO y hay instancias, proceso va a BLOCKED
     // Busco una instancia disponible (pid_actual = -1)
    t_instancia_io* instancia_disponible = NULL;
    for(int i = 0; i < list_size(io->instancias_io); i++) {
        t_instancia_io* instancia = list_get(io->instancias_io, i);
        if(instancia->pid_actual == -1) {
            instancia_disponible = instancia;
            break;
        }
    }

    // Marco la instancia como ocupada
    instancia_disponible->pid_actual = pid;
    io->instancias_disponibles--;
    
    log_info(logger_kernel, "## PID (%d) Pasa del estado %s al estado %s", pid, "EXECUTING", "BLOCKED (esperando finalizacion de IO)");
    log_info(logger_kernel, "## PID (%d) - Bloqueado por IO: %s",pid, nombre_io);

    log_info(logger_kernel, "[IO-SOLICITUD] Enviando solicitud a %s (PID: %d, tiempo: %d)", nombre_io, pid, tiempo_io);
    // Creo la solicitud
    t_solicitud_io solicitud;
    solicitud.pid = pid;
    solicitud.tiempo = tiempo_io;
    // Serializo y envío
    t_buffer* buffer = serializar_solicitud_io(&solicitud);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_SOLICITUD_IO, buffer);
    
    enviar_paquete(instancia_disponible->socket_io, paquete);

    mover_pcb_a_blocked_desde_executing(pid);

    }
   
    
    
    
    
    pthread_mutex_unlock(&mutex_io);

    return 0;
    }
   


    