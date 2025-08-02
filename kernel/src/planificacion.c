#include "planificacion.h"



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Globales

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

u_int32_t grado_multiprogramacion = 0;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Colas Y listas de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


t_list* lista_new = NULL;
t_list* lista_ready = NULL;
t_list* lista_exit = NULL;
t_list* lista_blocked = NULL;
t_list* lista_executing = NULL;
t_list* lista_suspready = NULL;
t_list* lista_suspblocked = NULL;
t_list* lista_blocked_io = NULL;

t_list* lista_cpu = NULL;   
t_list* lista_io = NULL;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                            Semaforos

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pthread_mutex_t mutex_io = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cpu = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_lista_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_exit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_executing = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_blocked_io = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pid_counter = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_grado_multiprogramacion = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_suspblocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_suspready = PTHREAD_MUTEX_INITIALIZER;


sem_t sem_largo_plazo;
sem_t sem_corto_plazo;
sem_t sem_cpu_disponible;
sem_t sem_memoria_disponible;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones auxiliares a estructuras

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_blocked_io* crear_blocked_io(u_int32_t pid, char* nombre_io, u_int32_t tiempo_io){
    t_blocked_io* blocked_io = malloc(sizeof(t_blocked_io));
    blocked_io->pid = pid;
    blocked_io->nombre_io = strdup(nombre_io); // Usamos strdup para hacer una copia profunda
    blocked_io->tiempo_io = tiempo_io;
    return blocked_io;
}

bool comparar_pid(void* elemento, void* pid) {
    t_pcb* pcb = (t_pcb*)elemento;
    u_int32_t pid_pcb = pcb->pid;
    return pid_pcb == *(u_int32_t*)pid;
}

bool tiene_menor_estimacion(void* pcb1_void, void* pcb2_void) {
    t_pcb* pcb1 = (t_pcb*)pcb1_void;
    t_pcb* pcb2 = (t_pcb*)pcb2_void;
    return pcb1->proxima_estimacion < pcb2->proxima_estimacion;
}


int actualizar_estado_pcb(t_pcb* pcb, estado_pcb estado) {

    //Actualizo el tiempo en milisegundos del estado actual
    estado_pcb estado_actual= pcb->estado;

    pthread_mutex_lock(&pcb->mutex_cambio_estado);

    //-----------------------------------------------------------------------------------------------------------//
    //Obtengo el tiempo transcurrido en el estado anterior
    int64_t tiempo_en_ms = temporal_gettime(pcb->cronometro_estado);
    pcb->metricas_tiempo[estado_actual].tiempo_acumulado += tiempo_en_ms;
    //Reinicio el cronometro para el nuevo estado
    temporal_destroy(pcb->cronometro_estado);
    pcb->cronometro_estado = temporal_create();
   
    //Actualizo el contador de estado
    pcb->metricas_estado[estado].contador++;
    
    //Actualizo el estado actual del proceso
    pcb->estado = estado;

    pthread_mutex_unlock(&pcb->mutex_cambio_estado);

    //Loggeo el cambio de estado
    log_cambio_estado(pcb, estado_actual, estado);

    return 0;
}

t_blocked_io* buscar_proceso_en_blocked_io(int socket_io) {
    t_io* io = buscar_io_por_socket_unsafe(socket_io);
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

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                            Funciones para PIDs

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

bool _encontrar_pcb_por_pid(void* elemento, void* pid_a_comparar) {
    t_pcb* pcb = (t_pcb*)elemento;
    return pcb->pid == *(u_int32_t*)pid_a_comparar;
}


u_int32_t obtener_pid_siguiente() {
    pthread_mutex_lock(&mutex_pid_counter);
    u_int32_t pid_actual = pid_counter;
    pid_counter++;
    pthread_mutex_unlock(&mutex_pid_counter);
    return pid_actual;
}

u_int32_t obtener_pid_actual() {
    pthread_mutex_lock(&mutex_pid_counter);
    u_int32_t pid_actual = pid_counter;
    pthread_mutex_unlock(&mutex_pid_counter);
    return pid_actual;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                            Funciones de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


algoritmos_de_planificacion obtener_algoritmo_de_planificacion(char* algoritmo) {
    if(strcmp(algoritmo, "FIFO") == 0) {
        return FIFO;
    } else if(strcmp(algoritmo, "PMCP") == 0) {
        return PMCP;
    } else if(strcmp(algoritmo, "SJF") == 0) {
        return SJF_SIN_DESALOJO;
    } else if(strcmp(algoritmo, "SRT") == 0)   {
        return SJF_CON_DESALOJO;
    }
    log_error(logger_kernel, "Algoritmo de planificacion '%s' no reconocido. Usando FIFO por defecto.", algoritmo);
    return FIFO;
}

void planificar_proceso_inicial(char* archivo_pseudocodigo, u_int32_t tamanio_proceso) {
    t_pcb* pcb = inicializar_pcb(obtener_pid_siguiente(), archivo_pseudocodigo, tamanio_proceso, kernel_configs.estimacioninicial);
    agregar_a_new(pcb);
    //Loggear ## (<PID>) Se crea el proceso - Estado: NEW
    log_creacion_proceso(pcb);
}

void inicializar_listas_y_sem() {
    lista_new = list_create();
    lista_ready = list_create();
    lista_exit = list_create();
    lista_blocked  = list_create();
    lista_executing = list_create();
    lista_suspready = list_create();
    lista_suspblocked = list_create();
    lista_blocked_io = list_create();
    lista_cpu = list_create();
    lista_io = list_create();
    sem_init(&sem_largo_plazo, 0, 0);
    sem_init(&sem_corto_plazo, 0, 0);
    sem_init(&sem_cpu_disponible, 0, 0);
    sem_init(&sem_memoria_disponible, 0, 0);
}

int recibir_mensaje_cpu(int socket_cpu) {
    //TODO> evaluar todos los posibles paquetes que puedo recibir
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
            break;
        default:
            log_error(logger_kernel, "Codigo de operacion invalido recibido de CPU");
            return -1;
    }
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones para mover entre estados

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void agregar_a_new(t_pcb* pcb) {
    algoritmos_de_planificacion algoritmo = obtener_algoritmo_de_planificacion(kernel_configs.ingreasoaready);
    if(algoritmo == FIFO) {
        pthread_mutex_lock(&mutex_lista_new);
        list_add(lista_new, pcb);
        pthread_mutex_unlock(&mutex_lista_new);
        log_creacion_proceso(pcb);
        sem_post(&sem_largo_plazo); //Aviso al planificador largo plazo que hay un proceso en new
    } else if(algoritmo == PMCP) {
        pthread_mutex_lock(&mutex_lista_new);
        list_add_sorted(lista_new, pcb, es_mas_chico);
        pthread_mutex_unlock(&mutex_lista_new);
        log_creacion_proceso(pcb);
        sem_post(&sem_largo_plazo); //Aviso al planificador largo plazo que hay un proceso en new
    }
}


void mover_a_ready(t_pcb* pcb) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d a READY.", pcb->pid); // DEBUG_LOG
    algoritmos_de_planificacion algoritmo = obtener_algoritmo_de_planificacion(kernel_configs.ingreasoaready);
    algoritmos_de_planificacion algoritmocorto = obtener_algoritmo_de_planificacion(kernel_configs.cortoplazo);
    if(algoritmo == FIFO) {
        pthread_mutex_lock(&mutex_lista_ready);
        list_add(lista_ready, pcb);
        pthread_mutex_unlock(&mutex_lista_ready);
        actualizar_estado_pcb(pcb, READY);
        sem_post(&sem_corto_plazo);
    } else if(algoritmo == PMCP) {
        pthread_mutex_lock(&mutex_lista_ready);
        list_add_sorted(lista_ready, pcb, es_mas_chico);
        pthread_mutex_unlock(&mutex_lista_ready);
        actualizar_estado_pcb(pcb, READY);
        sem_post(&sem_corto_plazo);
    }
    
    if (algoritmocorto == SJF_CON_DESALOJO) {
        //Me fijo si hay una cpu disponible
        if(sem_trywait(&sem_cpu_disponible) == 0) {
            //Si hay cpu disponible, no hago nada. Devuelvo el semaforo
            sem_post(&sem_cpu_disponible);
        } else {
            //Si no hay CPU disponible, tengo que fijarme si el PCB que entro a ready tiene
            //Una estimacion de CPU menor a los que se estan ejecutando


            //Itero sobre la lista de executing
            pthread_mutex_lock(&mutex_lista_executing);

            if(list_is_empty(lista_executing)) {
                pthread_mutex_unlock(&mutex_lista_executing);
                return;
            }

            t_pcb* pcb_a_desalojar = NULL;
            double rafaga_restante_maxima = 0.0;


            for(int i = 0; i < list_size(lista_executing); i++) {
                t_pcb* pcb_ejecutando = list_get(lista_executing, i);

                //Calculo cuanto tiempo ha estado ejecutando
                int64_t tiempo_ejecutando_ms = temporal_gettime(pcb_ejecutando->cronometro_estado);
                double rafaga_restante = pcb_ejecutando->estimacion_rafaga_anterior - tiempo_ejecutando_ms;

                //Comparo y guardo el maximo, y el indice del PCB con el maximo.

                if(rafaga_restante > rafaga_restante_maxima) {
                    rafaga_restante_maxima = rafaga_restante;
                    pcb_a_desalojar = pcb_ejecutando;
                }
            }
            //ahora veo si desalojar
            if(pcb_a_desalojar != NULL && pcb->proxima_estimacion < rafaga_restante_maxima) {
                //Desalojo
                log_desalojo(pcb_a_desalojar);
                pthread_mutex_unlock(&mutex_lista_executing);
                mover_executing_a_ready(pcb_a_desalojar->pid);
            } else {
                pthread_mutex_unlock(&mutex_lista_executing);
            }
        
        }
    
    }

}

int mover_a_exit(t_pcb* pcb) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d a EXIT.", pcb->pid); // DEBUG_LOG
    
    actualizar_estado_pcb(pcb, EXIT);
    log_fin_proceso(pcb);
    log_metricas_estado(pcb);

    // Mensajes a memoria, esto lo va a verificar el planificador largo plazo en cada iteracion

    //list_add(lista_exit, pcb); //Solo iran a la lista de exit los que terminen por alguna interrupcion 
    //Aca solo actualizo el estado. Luego el hilo planificador de largo plazo va revisar la lista de ready lo que esten en estado exit
    //Va a sacarlos de la lista de ready y liberar la memoria del PCB.;
    

    return 0;
}

int mover_a_suspready(t_pcb* pcb) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d a SUSP_READY.", pcb->pid); // DEBUG_LOG
    algoritmos_de_planificacion algoritmo = obtener_algoritmo_de_planificacion(kernel_configs.ingreasoaready);
    if(algoritmo == FIFO) {
        pthread_mutex_lock(&mutex_lista_suspready);
        list_add(lista_suspready, pcb);
        pthread_mutex_unlock(&mutex_lista_suspready);
        actualizar_estado_pcb(pcb, SUSP_READY);
        sem_post(&sem_largo_plazo);
    } else if(algoritmo == PMCP) {
        pthread_mutex_lock(&mutex_lista_suspready);
        list_add_sorted(lista_suspready, pcb, es_mas_chico);
        pthread_mutex_unlock(&mutex_lista_suspready);
        actualizar_estado_pcb(pcb, SUSP_READY);
        sem_post(&sem_largo_plazo);
    }

    return 0;
}


int mover_executing_a_blockedio(u_int32_t pid, char* nombre_io, u_int32_t tiempo_io) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de EXECUTING a BLOCKED (IO).", pid); // DEBUG_LOG
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_executing);
    
    // Buscar el PCB en la lista
    for(int i = 0; i < list_size(lista_executing); i++) {
        t_pcb* pcb_temp = list_get(lista_executing, i);
        if(pcb_temp->pid == pid) {
            pcb_encontrado = pcb_temp;
            //Tambien lo muevo a la lista de blocked general
            //Actualizo el estado del proceso
            pthread_mutex_lock(&mutex_lista_blocked);
            list_add(lista_blocked, pcb_encontrado);
            pthread_mutex_unlock(&mutex_lista_blocked);
            actualizar_metricas_sjf(pcb_encontrado);
            actualizar_estado_pcb(pcb_encontrado, BLOCKED);

            //Llamo a la planificacion mediano plazo
            pthread_t thread_planificacion_mediano_plazo;
            pthread_create(&thread_planificacion_mediano_plazo, NULL, llamar_planificacion_mediano_plazo, (void*)pcb_encontrado);
            pthread_detach(thread_planificacion_mediano_plazo);

            //Antes de removerlo, lo muevo a blocked_io

            pthread_mutex_lock(&mutex_lista_blocked_io);
            t_blocked_io* blocked_io = crear_blocked_io(pcb_encontrado->pid, nombre_io, tiempo_io);
            list_add(lista_blocked_io, blocked_io);
            pthread_mutex_unlock(&mutex_lista_blocked_io);

            // Remover el elemento de la cola en la posición i
            list_remove(lista_executing, i);
            break;
        }
    }
    
    pthread_mutex_unlock(&mutex_lista_executing);

    //Tengo que avisar al CPU que el proceso se bloqueo
    if(pcb_encontrado != NULL) {
        t_cpu_en_kernel* cpu = obtener_cpu_por_pid(pcb_encontrado->pid);
        if(cpu != NULL) {
            t_interrupcion* interrupcion = malloc(sizeof(t_interrupcion));
            interrupcion->pid = pcb_encontrado->pid;
            interrupcion->pc = pcb_encontrado->pc;
            interrupcion->motivo = INTERRUPCION_BLOQUEO;
            t_buffer* buffer = serializar_interrupcion(interrupcion);
            t_paquete* paquete = empaquetar_buffer(PAQUETE_INTERRUPCION, buffer);
            enviar_paquete(cpu->socket_cpu_interrupt, paquete); //Esta funcion ya libera el paquete despues de enviarlo
            free(interrupcion);
            liberar_cpu(cpu);
        }
    } else {
        log_error(logger_kernel, "Inconsistencia: PID %d no encontrado en lista_executing", pid);
        return -1;
    }
    

    return 0;
}

int mover_executing_a_exit(u_int32_t pid) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de EXECUTING a EXIT.", pid); // DEBUG_LOG
    t_pcb* pcb_a_mover = NULL;
    pthread_mutex_lock(&mutex_lista_executing);

    for(int i = 0; i < list_size(lista_executing); i++) {
        t_pcb* pcb_temp = list_get(lista_executing, i);
        if(pcb_temp->pid == pid) {
            actualizar_metricas_sjf(pcb_temp);
            pcb_a_mover = list_remove(lista_executing, i);
            break;
        }
    }
    
    pthread_mutex_unlock(&mutex_lista_executing);

    if(pcb_a_mover == NULL) {
        log_error(logger_kernel, "Inconsistencia: PID %d no encontrado en lista_executing", pid);
        return -1;
    }

    t_cpu_en_kernel* cpu = obtener_cpu_por_pid(pcb_a_mover->pid);
    if(cpu != NULL) {
        t_interrupcion* interrupcion = malloc(sizeof(t_interrupcion));
        interrupcion->pid = pcb_a_mover->pid;
        interrupcion->pc = pcb_a_mover->pc;
        interrupcion->motivo = INTERRUPCION_FIN_EJECUCION;
        t_buffer* buffer = serializar_interrupcion(interrupcion);
        t_paquete* paquete = empaquetar_buffer(PAQUETE_INTERRUPCION, buffer);
        enviar_paquete(cpu->socket_cpu_interrupt, paquete); //Esta funcion ya libera el paquete despues de enviarlo
        free(interrupcion);
        liberar_cpu(cpu);
    }

    mover_a_exit(pcb_a_mover);

    return 0;
}

int mover_executing_a_blocked(u_int32_t pid) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de EXECUTING a BLOCKED.", pid); // DEBUG_LOG
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_executing);

    for(int i = 0; i < list_size(lista_executing); i++) {
        t_pcb* pcb_temp = list_get(lista_executing, i);
        if(pcb_temp->pid == pid) {
            pcb_encontrado = pcb_temp;
            //Antes de removerlo, lo muevo a blocked
            pthread_mutex_lock(&mutex_lista_blocked);
            list_add(lista_blocked, pcb_encontrado);
            pthread_mutex_unlock(&mutex_lista_blocked);
            //Actualizo el estado del proceso
            actualizar_metricas_sjf(pcb_encontrado);
            actualizar_estado_pcb(pcb_encontrado, BLOCKED);


            //Llamo a la planificacion mediano plazo
            pthread_t thread_planificacion_mediano_plazo;
            pthread_create(&thread_planificacion_mediano_plazo, NULL, llamar_planificacion_mediano_plazo, (void*)pcb_encontrado);
            pthread_detach(thread_planificacion_mediano_plazo);

            list_remove(lista_executing, i);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_executing);

    //Tengo que avisar al CPU que el proceso se bloqueo
    if(pcb_encontrado != NULL) {
        t_cpu_en_kernel* cpu = obtener_cpu_por_pid(pcb_encontrado->pid);
        if(cpu != NULL) {
            t_interrupcion* interrupcion = malloc(sizeof(t_interrupcion));
            interrupcion->pid = pcb_encontrado->pid;
            interrupcion->pc = pcb_encontrado->pc;
            interrupcion->motivo = INTERRUPCION_BLOQUEO;
            t_buffer* buffer = serializar_interrupcion(interrupcion);
            t_paquete* paquete = empaquetar_buffer(PAQUETE_INTERRUPCION, buffer);
            enviar_paquete(cpu->socket_cpu_interrupt, paquete); //Esta funcion ya libera el paquete despues de enviarlo
            free(interrupcion);
            liberar_cpu(cpu);
        }
    } else {
        log_error(logger_kernel, "Inconsistencia: PID %d no encontrado en lista_executing", pid);
        return -1;
    }
    

    return 0;
}

int mover_executing_a_ready(u_int32_t pid) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de EXECUTING a READY.", pid); // DEBUG_LOG
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_executing);
    for(int i = 0; i < list_size(lista_executing); i++) {
        t_pcb* pcb_temp = list_get(lista_executing, i);
        if(pcb_temp->pid == pid) {
            actualizar_metricas_sjf(pcb_temp);
            pcb_encontrado = list_remove(lista_executing, i);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_executing);

    if(pcb_encontrado == NULL) {
        log_error(logger_kernel, "Inconsistencia: PID %d no encontrado en lista_executing", pid);
        return -1;
    }
    //Tengo que avisar al CPU que el proceso no esta mas en executing
    t_cpu_en_kernel* cpu = obtener_cpu_por_pid(pcb_encontrado->pid);
    if(cpu != NULL) {
        t_interrupcion* interrupcion = malloc(sizeof(t_interrupcion));
        interrupcion->pid = pcb_encontrado->pid;
        interrupcion->pc = pcb_encontrado->pc;
        interrupcion->motivo = INTERRUPCION_FIN_EJECUCION;
        t_buffer* buffer = serializar_interrupcion(interrupcion);
        t_paquete* paquete = empaquetar_buffer(PAQUETE_INTERRUPCION, buffer);
        enviar_paquete(cpu->socket_cpu_interrupt, paquete); //Esta funcion ya libera el paquete despues de enviarlo
        //liberar_cpu(cpu);
        
    }

    mover_a_ready(pcb_encontrado);

    return 0;
}

int mover_blocked_a_ready(u_int32_t pid) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de BLOCKED a READY.", pid); // DEBUG_LOG
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_blocked);
    for(int i = 0; i < list_size(lista_blocked); i++) {
        pcb_encontrado = list_get(lista_blocked, i);
        if(pcb_encontrado->pid == pid) {
            pcb_encontrado = list_remove(lista_blocked, i);
            mover_a_ready(pcb_encontrado);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_blocked);
    return 0;
}

int mover_blockedio_a_exit(u_int32_t pid) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de BLOCKED_IO a EXIT.", pid); // DEBUG_LOG
    
    //Primero busco el pcb en la lista de blocked_io y lo remuevo

    t_blocked_io* pcb_blocked_removido = NULL;
    pthread_mutex_lock(&mutex_lista_blocked_io);
    for(int i = 0; i < list_size(lista_blocked_io); i++) {
        t_blocked_io* pcb_blocked_temp = list_get(lista_blocked_io, i);
        if(pcb_blocked_temp->pid == pid) {
            //si lo encuentro, lo remuevo de la lista
            pcb_blocked_removido = list_remove(lista_blocked_io, i);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_blocked_io);

    if(pcb_blocked_removido == NULL) {
        // Esto sería un estado inconsistente. El proceso debería estar aquí.
        log_error(logger_kernel, "Inconsistencia: PID %d no encontrado en lista_blocked_io", pid);
        return -1; // Error
    }
    //Una vez que lo saque de la lista, libero la memoria
    free(pcb_blocked_removido->nombre_io);
    free(pcb_blocked_removido);

    //Segundo lo tengo que remover de la lista general de blocked
    
        //Antes de removerlo, lo muevo a exit
    t_pcb* pcb_a_mover = NULL;
    pthread_mutex_lock(&mutex_lista_blocked);
    for(int i = 0; i < list_size(lista_blocked); i++) {
        t_pcb* pcb_temp = list_get(lista_blocked, i);
        if(pcb_temp->pid == pid) {
            //Lo remuevo de la lista de blocked
            pcb_a_mover = list_remove(lista_blocked, i);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_blocked);

    if(pcb_a_mover == NULL) {
        log_error(logger_kernel, "Inconsistencia: PID %d no encontrado en lista_blocked", pid);
        return -1; // Error
    }
    
    log_info(logger_kernel, "[IO] Moviendo PID %d a exit", pid);
    mover_a_exit(pcb_a_mover);

    return 0;
    
}


int mover_blocked_a_exit(u_int32_t pid){
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de BLOCKED a EXIT.", pid); // DEBUG_LOG
    t_pcb* pcb_a_mover = NULL;
    
    // Paso 1: Buscar y remover el PCB de la lista de bloqueados de forma atómica.
    pthread_mutex_lock(&mutex_lista_blocked);
    for(int i = 0; i < list_size(lista_blocked); i++) {
        t_pcb* pcb_temp = list_get(lista_blocked, i);
        if(pcb_temp->pid == pid) {
            pcb_a_mover = list_remove(lista_blocked, i);
            break; 
        }
    }
    pthread_mutex_unlock(&mutex_lista_blocked);

    // Paso 2: Si encontramos el PCB, lo procesamos fuera de la sección crítica.
    if(pcb_a_mover != NULL) {
        log_info(logger_kernel, "[PLANIFICADOR] Moviendo PID %d de BLOCKED a EXIT.", pid);
        mover_a_exit(pcb_a_mover);
    } else {
        log_error(logger_kernel, "[PLANIFICADOR] Inconsistencia: Se intentó mover a EXIT el PID %d desde BLOCKED, pero no fue encontrado.", pid);
        return -1;
    }

    return 0;
}

int mover_blocked_a_suspblocked(u_int32_t pid) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de BLOCKED a SUSP_BLOCKED.", pid); // DEBUG_LOG
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_blocked);
    for(int i = 0; i < list_size(lista_blocked); i++) {
        t_pcb* pcb_temp = list_get(lista_blocked, i);
        if(pcb_temp->pid == pid) {
            //Aviso a memoria para que pase el pcb a swap
            int socket_memoria = kernel_conectar_a_memoria();
            if(socket_memoria == -1) {
                log_error(logger_kernel, "Error al conectar con memoria");
                return -1;
            }
            enviar_handshake(socket_memoria, -2);
            t_paquete* paquete = empaquetar_buffer(PAQUETE_SUSPENDER_PROCESO, serializar_pcb(pcb_encontrado));
            enviar_paquete(socket_memoria, paquete);
            if(recibir_bool(socket_memoria)) {
                log_info(logger_kernel, "Memoria liberada correctamente");
                pcb_encontrado = list_remove(lista_blocked, i);
                sem_post(&sem_memoria_disponible); //Ticket de memoria libre
            } else {
                log_error(logger_kernel, "Error al liberar memoria");
            }
            close(socket_memoria);
            //Termina conexion con memoria
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_blocked);

    if(pcb_encontrado == NULL) {
        log_error(logger_kernel, "Inconsistencia: PID %d no encontrado en lista_blocked", pid);
        return -1;
    } else {
        pthread_mutex_lock(&mutex_lista_suspblocked);
        list_add(lista_suspblocked, pcb_encontrado);
        pthread_mutex_unlock(&mutex_lista_suspblocked);
        actualizar_estado_pcb(pcb_encontrado, SUSP_BLOCKED);


    }
    return 0;
};

int mover_suspblocked_a_suspready(u_int32_t pid) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de SUSP_BLOCKED a SUSP_READY.", pid); // DEBUG_LOG
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_suspblocked);
    for(int i = 0; i < list_size(lista_suspblocked); i++) {
        t_pcb* pcb_temp = list_get(lista_suspblocked, i);
        if(pcb_temp->pid == pid) {
            pcb_encontrado = list_remove(lista_suspblocked, i);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_suspblocked);

    if(pcb_encontrado == NULL) {
        log_error(logger_kernel, "Inconsistencia: PID %d no encontrado en lista_suspblocked", pid);
        return -1;
    } else {
        mover_a_suspready(pcb_encontrado);
    }
    return 0;
};

int mover_suspready_a_ready(u_int32_t pid) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de SUSP_READY a READY.", pid); // DEBUG_LOG
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_suspready);
    for(int i = 0; i < list_size(lista_suspready); i++) {
        t_pcb* pcb_temp = list_get(lista_suspready, i);
        if(pcb_temp->pid == pid) {
            pcb_encontrado = list_remove(lista_suspready, i);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_suspready);

    if(pcb_encontrado == NULL) {
        log_error(logger_kernel, "Inconsistencia: PID %d no encontrado en lista_suspready", pid);
        return -1;
    } else {
        mover_a_ready(pcb_encontrado);
    }
    return 0;
};

int mover_ready_a_executing(u_int32_t pid) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de READY a EXECUTING.", pid); // DEBUG_LOG
    algoritmos_de_planificacion algoritmo = obtener_algoritmo_de_planificacion(kernel_configs.cortoplazo);
    t_pcb* pcb = NULL;
    pthread_mutex_lock(&mutex_lista_ready);
    for(int i = 0; i < list_size(lista_ready); i++) {
        t_pcb* pcb_temp = list_get(lista_ready, i);
        if(pcb_temp->pid == pid) {
            pcb = list_remove(lista_ready, i);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_ready);

    if(pcb == NULL) {
        log_error(logger_kernel, "ERROR CRÍTICO: PID %d no encontrado en lista_ready para mover a EXECUTING", pid);
        return -1;
    } else {

        if(algoritmo == FIFO) {
            pthread_mutex_lock(&mutex_lista_executing);
            list_add(lista_executing, pcb);
            pthread_mutex_unlock(&mutex_lista_executing);
            actualizar_estado_pcb(pcb, RUNNING);

        } else if(algoritmo == SJF_SIN_DESALOJO) {
            pcb->estimacion_rafaga_anterior = pcb->proxima_estimacion;
            pthread_mutex_lock(&mutex_lista_executing);
            list_add(lista_executing, pcb);
            pthread_mutex_unlock(&mutex_lista_executing);
            actualizar_estado_pcb(pcb, RUNNING);

        } else if(algoritmo == SJF_CON_DESALOJO) {
            pcb->estimacion_rafaga_anterior = pcb->proxima_estimacion;
            pthread_mutex_lock(&mutex_lista_executing);
            list_add(lista_executing, pcb);
            pthread_mutex_unlock(&mutex_lista_executing);
            actualizar_estado_pcb(pcb, RUNNING);
        }
    }

    return 0;
}

int mover_suspblocked_a_exit(u_int32_t pid) {
    log_debug(logger_kernel, "[Transición] Intentando mover PID %d de SUSP_BLOCKED a EXIT.", pid); // DEBUG_LOG
    t_pcb* pcb_a_mover = NULL;
    
    pthread_mutex_lock(&mutex_lista_suspblocked);

    for(int i = 0; i < list_size(lista_suspblocked); i++) {
        t_pcb* pcb_temp = list_get(lista_suspblocked, i);
        if(pcb_temp->pid == pid) {
            pcb_a_mover = list_remove(lista_suspblocked, i);
            break;
        }
    }
        pthread_mutex_unlock(&mutex_lista_suspblocked);

    if(pcb_a_mover != NULL) {
        mover_a_exit(pcb_a_mover);
    } else {
        log_error(logger_kernel, "Inconsistencia: PID %d no encontrado en lista_suspblocked", pid);
        return -1;
    }

    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones auxiliares para listas y colas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_pcb* peek_lista_new() {
    pthread_mutex_lock(&mutex_lista_new);

    if (list_is_empty(lista_new)) {
        pthread_mutex_unlock(&mutex_lista_new);
        return NULL;
    }

    t_pcb* pcb = list_get(lista_new, 0);
    pthread_mutex_unlock(&mutex_lista_new);
    return pcb;
}   

t_pcb* peek_lista_suspready() {
    pthread_mutex_lock(&mutex_lista_suspready);

    if (list_is_empty(lista_suspready)) {
        pthread_mutex_unlock(&mutex_lista_suspready);
        return NULL;
    }

    t_pcb* pcb = list_get(lista_suspready, 0);
    pthread_mutex_unlock(&mutex_lista_suspready);
    return pcb;
}

t_pcb* peek_lista_ready() {
    pthread_mutex_lock(&mutex_lista_ready);

    if (list_is_empty(lista_ready)) {
        pthread_mutex_unlock(&mutex_lista_ready);
        return NULL;
    }

    t_pcb* pcb = list_get(lista_ready, 0);
    pthread_mutex_unlock(&mutex_lista_ready);
    return pcb;
}   

t_pcb* obtener_pcb_de_lista_new() {
    pthread_mutex_lock(&mutex_lista_new);

    if (list_is_empty(lista_new)) {
        pthread_mutex_unlock(&mutex_lista_new);
        return NULL;
    }

    t_pcb* pcb = list_remove(lista_new, 0);
    log_debug(logger_kernel, "[Lista NEW] PID %d removido de la lista.", pcb->pid); // DEBUG_LOG
    pthread_mutex_unlock(&mutex_lista_new);
    return pcb;
}

t_pcb* obtener_pcb_de_lista_suspready() {
    pthread_mutex_lock(&mutex_lista_suspready);

    if (list_is_empty(lista_suspready)) {
        pthread_mutex_unlock(&mutex_lista_suspready);
        return NULL;
    }

    t_pcb* pcb = list_remove(lista_suspready, 0);
    log_debug(logger_kernel, "[Lista SUSP_READY] PID %d removido de la lista.", pcb->pid); // DEBUG_LOG

    pthread_mutex_unlock(&mutex_lista_suspready);
    return pcb;
}

t_pcb* obtener_pcb_de_lista_executing(u_int32_t pid) {
    t_pcb* pcb_encontrado = NULL;
    pthread_mutex_lock(&mutex_lista_executing);
    for(int i = 0; i < list_size(lista_executing); i++) {
        t_pcb* pcb = list_get(lista_executing, i);
        if(pcb->pid == pid) {
            pcb_encontrado = pcb;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_executing);
    return pcb_encontrado;
}


int sacar_de_blockedio(t_blocked_io* io_a_sacar) {
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
    // Nos aseguramos de solo remover el nodo, sin liberar el dato.
    // La liberación se hará de forma controlada en la función que llamó a esta.
    if (indice_a_eliminar != -1) {
    list_remove(lista_blocked_io, indice_a_eliminar);
    }
    pthread_mutex_unlock(&mutex_lista_blocked_io);
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones auxiliares de busqueda

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


bool comparar_socket_io(void* elemento, void* socket_a_comparar) {
    int socket = (int)(intptr_t)socket_a_comparar;
    t_io* io = (t_io*)elemento;
    //Itero la lista de instancias de este IO, comparo el socket que recibi con el socket de cada instancia

    for(int i = 0; i < list_size(io->instancias_io); i++) {
        t_instancia_io* instancia_io = list_get(io->instancias_io, i);
        if(instancia_io->socket_io == socket) {
            return true;
        }
    }
    return false;
}
 
 bool es_mas_chico(void* elemento, void* otro_elemento) {
    t_pcb* pcb = (t_pcb*)elemento;
    t_pcb* otro_pcb = (t_pcb*)otro_elemento;
    return pcb->tamanio_proceso < otro_pcb->tamanio_proceso;
 }

t_io* buscar_io_por_socket_unsafe(int socket_io){
    return list_find_con_param(lista_io, comparar_socket_io, (void*)(intptr_t)socket_io);
}

t_io* buscar_io_por_socket(int socket_io){
    pthread_mutex_lock(&mutex_io);
    t_io* io = list_find_con_param(lista_io, comparar_socket_io, (void*)(intptr_t)socket_io);
    pthread_mutex_unlock(&mutex_io);
    return io;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para planificacion largo plazo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void mover_procesos_terminados() {
    pthread_mutex_lock(&mutex_lista_executing);
    for(int i = list_size(lista_executing) - 1; i >= 0; i--) {
        t_pcb* pcb = list_get(lista_executing, i);
        if(pcb != NULL && pcb->estado == EXIT) {
            t_pcb* pcb_a_mover = list_remove(lista_executing, i);   
            //Posible deadlock?
            pthread_mutex_lock(&mutex_lista_exit);
            list_add(lista_exit, pcb_a_mover);
            pthread_mutex_unlock(&mutex_lista_exit);
        }
    }
    pthread_mutex_unlock(&mutex_lista_executing);
}

void eliminar_procesos_en_exit(){
    pthread_mutex_lock(&mutex_lista_exit);
    for(int i = list_size(lista_exit) - 1; i >= 0; i--) {
        t_pcb* pcb = list_get(lista_exit, i);
        if(pcb->estado == EXIT) {
            // Aca debo asvisar a memoria, me da el ok y recien ahi borro el PCB
            int socket_memoria = kernel_conectar_a_memoria();
            if(socket_memoria == -1) {
                log_error(logger_kernel, "Error al conectar con memoria");
                close(socket_memoria);
                pthread_mutex_unlock(&mutex_lista_exit); // Liberar antes de salir por error
                return;
            }
            enviar_handshake(socket_memoria, -2);
            t_paquete* paquete = empaquetar_buffer(PAQUETE_ELIMINAR_PROCESO, serializar_pcb(pcb));
            enviar_paquete(socket_memoria, paquete);
            if(recibir_bool(socket_memoria)) {
                log_info(logger_kernel, "Memoria liberada correctamente");
                t_pcb* pcb_a_eliminar = list_remove(lista_exit, i);
                destruir_pcb(pcb_a_eliminar);
                sem_post(&sem_memoria_disponible); //Ticket de memoria libre
            } else {
                log_error(logger_kernel, "Error al liberar memoria");
        }
            close(socket_memoria);

        }
    }
    pthread_mutex_unlock(&mutex_lista_exit); // Liberar al final del flujo normal
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para planificacion corto plazo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_pcb* obtener_pcb_con_menor_estimacion() {
    pthread_mutex_lock(&mutex_lista_ready);

    if(list_is_empty(lista_ready)) {
        pthread_mutex_unlock(&mutex_lista_ready);
        log_error(logger_kernel, "No hay procesos en lista_ready");
        return NULL;
    }

    // ¡AGREGAR LOGS DE DIAGNÓSTICO!
    log_debug(logger_kernel, "[SJF] Lista READY tiene %d procesos", list_size(lista_ready));
    for(int i = 0; i < list_size(lista_ready); i++) {
        t_pcb* pcb = list_get(lista_ready, i);
        log_debug(logger_kernel, "[SJF] PID %d - Estimación: %f", pcb->pid, pcb->proxima_estimacion);
    }

    t_pcb* pcb_elegido= list_get(lista_ready, 0);
    int indice_elegido = 0;
    
    // ¡AGREGAR LOG INICIAL!
    log_debug(logger_kernel, "[SJF] Inicialmente elegido: PID %d (estimación: %f)", pcb_elegido->pid, pcb_elegido->proxima_estimacion);

    for(int i = 1; i < list_size(lista_ready); i++) {
        t_pcb* pcb_actual = list_get(lista_ready, i);

        if(pcb_actual->proxima_estimacion < pcb_elegido->proxima_estimacion) {  // ¡CAMBIAR <= por < para FIFO!
            // ¡AGREGAR LOG DE CAMBIO!
            log_debug(logger_kernel, "[SJF] Nuevo elegido: PID %d (estimación: %f) reemplaza a PID %d", 
                     pcb_actual->pid, pcb_actual->proxima_estimacion, pcb_elegido->pid);
            pcb_elegido = pcb_actual;
            indice_elegido = i;
        }
    }

    list_remove(lista_ready, indice_elegido);
    log_debug(logger_kernel, "[Lista READY] PID %d removido (menor estimación: %f).", pcb_elegido->pid, pcb_elegido->proxima_estimacion);
    pthread_mutex_unlock(&mutex_lista_ready);
    return pcb_elegido;
}
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Actualizacion de PCB

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void actualizar_pcb(u_int32_t pid, u_int32_t pc) {
    pthread_mutex_lock(&mutex_lista_blocked);
    pthread_mutex_lock(&mutex_lista_suspblocked);
    pthread_mutex_lock(&mutex_lista_ready);
    pthread_mutex_lock(&mutex_lista_suspready);

    bool encontrado = false;
    t_pcb* pcb_temporal = NULL;

    //Busco en blocked
    for(int i = 0; i < list_size(lista_blocked); i++) {
        pcb_temporal = list_get(lista_blocked, i);
        if(pcb_temporal->pid == pid) {
            pcb_temporal->pc = pc;
            encontrado = true;
            break;
        }
    }
    //Busco en suspblocked
    if(!encontrado) {
        for(int i = 0; i < list_size(lista_suspblocked); i++) {
            pcb_temporal = list_get(lista_suspblocked, i);
            if(pcb_temporal->pid == pid) {
                pcb_temporal->pc = pc;
                encontrado = true;
                break;
            }
        }
    }
    //Busco en ready
    if(!encontrado) {
        for(int i = 0; i < list_size(lista_ready); i++) {
            pcb_temporal = list_get(lista_ready, i);
            if(pcb_temporal->pid == pid) {
                pcb_temporal->pc = pc;
                encontrado = true;
                break;
            }
        }
    }   
    //Busco en suspready
    if(!encontrado) {
        for(int i = 0; i < list_size(lista_suspready); i++) {
            pcb_temporal = list_get(lista_suspready, i);
        if(pcb_temporal->pid == pid) {
            pcb_temporal->pc = pc;
            encontrado = true;
            break;
        }
        }
    }
    //Busco en exit
    if(!encontrado) {
        for(int i = 0; i < list_size(lista_exit); i++) {
            pcb_temporal = list_get(lista_exit, i);
            if(pcb_temporal->pid == pid) {
                pcb_temporal->pc = pc;
                encontrado = true;
                break;
            }
        }
    }
    pthread_mutex_unlock(&mutex_lista_suspready);
    pthread_mutex_unlock(&mutex_lista_ready);
    pthread_mutex_unlock(&mutex_lista_suspblocked);
    pthread_mutex_unlock(&mutex_lista_blocked);

    log_info(logger_kernel, "PCB actualizado para PID %d, pc: %d", pid, pc);

    if(!encontrado) {
        log_warning(logger_kernel, "Se recibió contexto para el PID %d, pero no se encontró en ninguna lista para actualizar.", pid);
    }
    
}

int actualizar_metricas_sjf(t_pcb* pcb) {
   int64_t rafaga_real_ms = temporal_gettime(pcb->cronometro_estado);
   pcb->rafaga_real_anterior = (double)rafaga_real_ms;

   //Calculo la nueva estimacion

   double alfa = kernel_configs.alfa;
   double nueva_estimacion = (alfa * pcb->rafaga_real_anterior + (1 - alfa) * pcb->estimacion_rafaga_anterior);
   pcb->proxima_estimacion = nueva_estimacion;

   return 0;
   
}



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para planificacion mediano plazo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void* llamar_planificacion_mediano_plazo(void* pcb){
    t_pcb* pcb_a_mover = (t_pcb*)pcb;

   int tiempo_suspension = kernel_configs.tiemposuspension;
   //Espero el tiempo de suspension
    usleep(tiempo_suspension * 1000);
    pthread_mutex_lock(&pcb_a_mover->mutex_cambio_estado);
   if(pcb_a_mover->estado == BLOCKED) {
    mover_blocked_a_suspblocked(pcb_a_mover->pid);
   }
   pthread_mutex_unlock(&pcb_a_mover->mutex_cambio_estado);
   return NULL;
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones manejo de t_cpu_en_kernel

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



t_cpu_en_kernel* buscar_cpu_por_id_unsafe(int id_cpu){
    for(int i = 0; i < list_size(lista_cpu); i++) {
        t_cpu_en_kernel* cpu = list_get(lista_cpu, i);
        if(cpu->id_cpu == id_cpu) {
            return cpu;
        }
    }
    return NULL;
}

t_cpu_en_kernel* obtener_cpu_libre(){
    pthread_mutex_lock(&mutex_cpu);

    for(int i = 0; i < list_size(lista_cpu); i++) {
        t_cpu_en_kernel* cpu = list_get(lista_cpu, i);
        if(cpu->esta_ocupada == false) {
            pthread_mutex_unlock(&mutex_cpu);
            return cpu;
        }
    }
    pthread_mutex_unlock(&mutex_cpu);
    return NULL;
};

int buscar_cpu_por_socket_unsafe(int socket) {
    for(int i = 0; i < list_size(lista_cpu); i++) {
        t_cpu_en_kernel* cpu = list_get(lista_cpu, i);
        if(cpu->socket_cpu_dispatch == socket || cpu->socket_cpu_interrupt == socket) {
            return i;
        }
    }
    return -1;
}

int asignar_pcb_a_cpu(t_pcb* pcb){
    t_cpu_en_kernel* cpu = obtener_cpu_libre();
    if(cpu == NULL) {
        log_debug(logger_kernel, "[PCP] No hay CPUs libres");
        return -1;
    }
    cpu->esta_ocupada = true;
    cpu->pid_actual = pcb->pid;

    //Creo la estructura que contiene el pid y el pc del proceso
    t_proceso_cpu* pcb_a_enviar = malloc(sizeof(t_proceso_cpu));
    pcb_a_enviar->pid = pcb->pid;
    pcb_a_enviar->pc = pcb->pc;
    //Lo serializo, lo empaqueto y lo envio
    t_buffer* buffer = serializar_proceso_cpu(pcb_a_enviar);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_PROCESO_CPU, buffer);
    enviar_paquete(cpu->socket_cpu_dispatch, paquete);

    free(pcb_a_enviar);

    mover_ready_a_executing(pcb->pid);

     log_debug(logger_kernel, "[PCP] Asignado! PID %d a CPU%d.", pcb->pid, cpu->id_cpu);

    return 0;
}

asignar_pcb_ya_removido_a_cpu(t_pcb* pcb){
    t_cpu_en_kernel* cpu = obtener_cpu_libre();
    if(cpu == NULL) {
        log_debug(logger_kernel, "[PCP] No hay CPUs libres");
        return -1;
    }
    cpu->esta_ocupada = true;
    cpu->pid_actual = pcb->pid;

    //Creo la estructura que contiene el pid y el pc del proceso
    t_proceso_cpu* pcb_a_enviar = malloc(sizeof(t_proceso_cpu));
    pcb_a_enviar->pid = pcb->pid;
    pcb_a_enviar->pc = pcb->pc;
    //Lo serializo, lo empaqueto y lo envio
    t_buffer* buffer = serializar_proceso_cpu(pcb_a_enviar);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_PROCESO_CPU, buffer);
    enviar_paquete(cpu->socket_cpu_dispatch, paquete);

    free(pcb_a_enviar);
    
    algoritmos_de_planificacion algoritmo = obtener_algoritmo_de_planificacion(kernel_configs.cortoplazo);
    
    if(algoritmo == SJF_SIN_DESALOJO) {
        pcb->estimacion_rafaga_anterior = pcb->proxima_estimacion;
        pthread_mutex_lock(&mutex_lista_executing);
        list_add(lista_executing, pcb);
        pthread_mutex_unlock(&mutex_lista_executing);
        actualizar_estado_pcb(pcb, RUNNING);

    } else if(algoritmo == SJF_CON_DESALOJO) {
        pcb->estimacion_rafaga_anterior = pcb->proxima_estimacion;
        pthread_mutex_lock(&mutex_lista_executing);
        list_add(lista_executing, pcb);
        pthread_mutex_unlock(&mutex_lista_executing);
        actualizar_estado_pcb(pcb, RUNNING);
    }

    log_debug(logger_kernel, "[PCP] Asignado! PID %d a CPU%d.", pcb->pid, cpu->id_cpu);

    return 0;
}

t_cpu_en_kernel* obtener_cpu_por_pid(u_int32_t pid) {
    t_cpu_en_kernel* cpu = NULL;
    pthread_mutex_lock(&mutex_cpu);
    for(int i = 0; i < list_size(lista_cpu); i++) {
        cpu = list_get(lista_cpu, i);
        if(cpu->pid_actual == pid) {
            pthread_mutex_unlock(&mutex_cpu);
            return cpu;
        }
    }
    pthread_mutex_unlock(&mutex_cpu);
    return NULL;
}

int liberar_cpu(t_cpu_en_kernel* cpu) {
    pthread_mutex_lock(&mutex_cpu);
    t_cpu_en_kernel* cpu_a_liberar = NULL;
    for(int i = 0; i < list_size(lista_cpu); i++) {
        cpu_a_liberar = list_get(lista_cpu, i);
        if(cpu->id_cpu == cpu_a_liberar->id_cpu) {
            cpu_a_liberar->esta_ocupada = false;
            cpu_a_liberar->pid_actual = -1;
            sem_post(&sem_cpu_disponible);
            break;
        } 
    }
    pthread_mutex_unlock(&mutex_cpu);
    
    return 0;
}

                                    /*/////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                    
                                                                            SYSCALLS
                                    
                                    /////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones auxiliares para las syscalls

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_io* buscar_io_por_nombre(char* nombre_buscado) {
    for(int i = 0; i < list_size(lista_io); i++) {
        t_io* io = list_get(lista_io, i);
        if(strcmp(io->nombre_io, nombre_buscado) == 0) {
            return io;
        }
    }
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
        log_solicitud_syscall(syscall->pid, "IO");
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
        log_solicitud_syscall(syscall->pid, "INIT_PROC");
        if(init_proc(archivo_pseudocodigo, (u_int32_t)atoi(tamanio_proceso)) == -1) {
            printf("Error al ejecutar syscall INIT_PROC\n");
            return -1;
        }
    } else if(strcmp(tipo_syscall, "EXIT") == 0) {
        log_solicitud_syscall(syscall->pid, "EXIT");
        exit_syscall(syscall->pid);
    } else if(strcmp(tipo_syscall, "DUMP_MEMORY") == 0) {
        log_solicitud_syscall(syscall->pid, "DUMP_MEMORY");
        dump_memory(syscall->pid);
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
    t_pcb* pcb = inicializar_pcb(obtener_pid_siguiente(), archivo_pseudocodigo, tamanio_proceso, kernel_configs.estimacioninicial);
    agregar_a_new(pcb);
    return 0;
} 

int io (char* nombre_io, u_int32_t tiempo_io, u_int32_t pid) {

    pthread_mutex_lock(&mutex_io);

    //Buscar IO en la lista de IO
    t_io* io = buscar_io_por_nombre(nombre_io);

    //Si no existe, proceso va a EXIT
    if(io == NULL) {
        log_info(logger_kernel, "No existe el IO %s", nombre_io);
        mover_executing_a_exit(pid);
        pthread_mutex_unlock(&mutex_io);
        return -1;
    }

    //Si existe pero no hay instancias, proceso va a BLOCKED IO (tambien va a blocked general, esto se hace en mover_executing_a_blockedio)
    if(io->instancias_disponibles == 0) {
        mover_executing_a_blockedio(pid, nombre_io, tiempo_io);
        //log_info(logger_kernel, "## PID (%d) - Bloqueado por IO: %s",pid, nombre_io); //Log obligatorio, no puedo usar funcion de logeo porque no conozco el pcb
        log_motivo_bloqueo(pid, nombre_io);
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
    
    log_info(logger_kernel, "## PID (%d) - Bloqueado por IO: %s",pid, nombre_io); //Log obligatorio, no puedo usar funcion de logeo porque no conozco el pcb

    log_info(logger_kernel, "[IO-SOLICITUD] Enviando solicitud a %s (PID: %d, tiempo: %d)", nombre_io, pid, tiempo_io);
    // Creo la solicitud
    t_solicitud_io solicitud;
    solicitud.pid = pid;
    solicitud.tiempo = tiempo_io;
    // Serializo y envío
    t_buffer* buffer = serializar_solicitud_io(&solicitud);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_SOLICITUD_IO, buffer);
    
    enviar_paquete(instancia_disponible->socket_io, paquete); 

    mover_executing_a_blocked(pid);

    }
   
    
    
    
    
    pthread_mutex_unlock(&mutex_io);

    return 0;
    }
   


int exit_syscall(u_int32_t pid) {
    t_pcb* pcb = NULL;
    pthread_mutex_lock(&mutex_lista_executing);
    for(int i = 0; i < list_size(lista_executing); i++) {
        pcb = list_get(lista_executing, i);
        if(pcb->pid == pid) {
            actualizar_estado_pcb(pcb, EXIT);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lista_executing);

    t_cpu_en_kernel* cpu = obtener_cpu_por_pid(pid);
    if(cpu != NULL) {
        liberar_cpu(cpu);               // Liberar CPU (patrón consistente)
    }
    sem_post(&sem_largo_plazo);         // Despertar PLP
    
    return 0;
}

int dump_memory(u_int32_t pid) {
    //Muevo proceso a blocked
    t_pcb* pcb = obtener_pcb_de_lista_executing(pid);
    mover_executing_a_blocked(pid);
    int socket_memoria = kernel_conectar_a_memoria();
    if(socket_memoria == -1) {
    log_error(logger_kernel, "Error al conectar con memoria");
    close(socket_memoria);
    //Si falla el proceso va de blocked a exit
    mover_blocked_a_exit(pid);
    return -1;
    }

    enviar_handshake(socket_memoria, -2);
    
    t_paquete* paquete = empaquetar_buffer(PAQUETE_DUMP_MEMORY, serializar_pcb(pcb));
    enviar_paquete(socket_memoria, paquete);
   
    if(recibir_bool(socket_memoria)) {
        log_info(logger_kernel, "DUMP MEMORY COMPLETO");
        pthread_mutex_lock(&pcb->mutex_cambio_estado); //Lo lockeo para que no pueda cambiar el estado mientras chequeo
        if(pcb->estado == BLOCKED) {
            pthread_mutex_unlock(&pcb->mutex_cambio_estado);
            mover_blocked_a_ready(pid);
        } else if(pcb->estado == SUSP_BLOCKED) {
            pthread_mutex_unlock(&pcb->mutex_cambio_estado);
            mover_suspblocked_a_suspready(pid);
        } else {
            pthread_mutex_unlock(&pcb->mutex_cambio_estado);
            log_error(logger_kernel, "PID %d en estado inesperado %s durante DUMP", pid, estado_pcb_to_string(pcb->estado));
        }
    } else {
        pthread_mutex_lock(&pcb->mutex_cambio_estado); //Lo lockeo para que no pueda cambiar el estado mientras chequeo
        if(pcb->estado == BLOCKED) {
            pthread_mutex_unlock(&pcb->mutex_cambio_estado);
            mover_blocked_a_exit(pid);
        } else if(pcb->estado == SUSP_BLOCKED) {
            pthread_mutex_unlock(&pcb->mutex_cambio_estado);
            mover_suspblocked_a_exit(pid);
        } else {
            pthread_mutex_unlock(&pcb->mutex_cambio_estado);
            log_error(logger_kernel, "PID %d en estado inesperado %s durante DUMP", pid, estado_pcb_to_string(pcb->estado));
        }

    }

    //Cierro conexion efimera
    close(socket_memoria);
    return 0;
   
    
}