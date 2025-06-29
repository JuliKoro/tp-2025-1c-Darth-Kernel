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
t_queue* cola_exit = NULL;
t_queue* cola_blocked = NULL;
t_queue* cola_executing = NULL;
t_queue* cola_susp_ready = NULL;
t_queue* cola_susp_blocked = NULL;
t_queue* cola_susp_blocked_io = NULL;
t_list* lista_io = NULL;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                            Semaforos

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pthread_mutex_t mutex_io = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_exit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_executing = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_susp_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_susp_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_susp_blocked_io = PTHREAD_MUTEX_INITIALIZER;
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
    cola_exit = queue_create();
    cola_blocked = queue_create();
    cola_executing = queue_create();
    cola_susp_ready = queue_create();
    cola_susp_blocked = queue_create();
    cola_susp_blocked_io = queue_create();
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
            manejar_syscall(syscall->syscall);
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

                                        Funciones para agregar a las colas

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

void agregar_pcb_a_cola_blocked(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_cola_blocked);
    queue_push(cola_blocked, pcb);
    pthread_mutex_unlock(&mutex_cola_blocked);
}

void agregar_pcb_a_cola_exit(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_cola_exit);
    queue_push(cola_exit, pcb);
    pthread_mutex_unlock(&mutex_cola_exit);
}

void agregar_pcb_a_cola_executing(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_cola_executing);
    queue_push(cola_executing, pcb);
    pthread_mutex_unlock(&mutex_cola_executing);
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

t_pcb* peek_cola_blocked() {
    pthread_mutex_lock(&mutex_cola_blocked);
    t_pcb* pcb = queue_peek(cola_blocked);
    pthread_mutex_unlock(&mutex_cola_blocked);
    return pcb;
}

t_pcb* peek_cola_exit() {
    pthread_mutex_lock(&mutex_cola_exit);
    t_pcb* pcb = queue_peek(cola_exit);
    pthread_mutex_unlock(&mutex_cola_exit);
    return pcb;
}

t_pcb* peek_cola_executing() {
    pthread_mutex_lock(&mutex_cola_executing);
    t_pcb* pcb = queue_peek(cola_executing);
    pthread_mutex_unlock(&mutex_cola_executing);
    return pcb;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para obtener pcb de las colas

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

t_pcb* obtener_pcb_de_cola_blocked() {
    pthread_mutex_lock(&mutex_cola_blocked);
    t_pcb* pcb = queue_pop(cola_blocked);
    pthread_mutex_unlock(&mutex_cola_blocked);
    return pcb;
}   

t_pcb* obtener_pcb_de_cola_exit() {
    pthread_mutex_lock(&mutex_cola_exit);
    t_pcb* pcb = queue_pop(cola_exit);
    pthread_mutex_unlock(&mutex_cola_exit);
    return pcb; 
}

t_pcb* obtener_pcb_de_cola_executing() {
    pthread_mutex_lock(&mutex_cola_executing);
    t_pcb* pcb = queue_pop(cola_executing);
    pthread_mutex_unlock(&mutex_cola_executing);    
    return pcb;
}



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para planificacion largo plazo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void mover_procesos_terminados() {
    int elementos_procesados = 0;
    int total_elementos = queue_size(cola_executing);

    while(elementos_procesados < total_elementos) {
        t_pcb* pcb = peek_cola_executing();

        if(pcb != NULL && pcb->estado == EXIT) {
            t_pcb* pcb_exit = obtener_pcb_de_cola_executing();
            agregar_pcb_a_cola_exit(pcb_exit);
        } else {
            t_pcb* pcb_temp = obtener_pcb_de_cola_executing();
            agregar_pcb_a_cola_executing(pcb_temp);
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


int manejar_syscall(char* syscall) {
    //Leo hasta el primer espacio, obtengo syscall
    char* tipo_syscall = strtok(syscall, " ");

    //Evaluo el tipo de syscall 

    if(strcmp(tipo_syscall, "IO") == 0) {
        //Leo hasta el siguiente espacio, obtengo nombre_io
        char* nombre_io = strtok(NULL, " ");
        //Leo hasta el siguiente espacio, obtengo tiempo_io
        char* tiempo_io = strtok(NULL, " ");
        //Llamo a la funcion io
        if(io(nombre_io, (u_int32_t)atoi(tiempo_io)) == -1) {
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

int io (char* nombre_io, u_int32_t tiempo_io) {
    //Buscar IO en la lista de IO
    t_io* io = buscar_io_por_nombre(nombre_io);
    //Si no existe, proceso va a EXIT
    if(io == NULL) {
        //TODO: enviar a exit
    }
    //Si existe, proceso va a BLOCKED

    //Se envia la peticion al IO
    //pthread_mutex_lock(&io->mutex_cola_blocked_io);
    //queue_push(io->cola_blocked_io, obtener_pid_actual()); // Esto esta mal
    pthread_mutex_unlock(&io->mutex_cola_blocked_io);
    //Se espera a que el IO finalice la operacion

    //Se actualiza la cola de ready

    //Se actualiza la cola de blocked

    //Se actualiza la cola de executing

    //Se actualiza la cola de exit

    //Se actualiza la cola de new   
}