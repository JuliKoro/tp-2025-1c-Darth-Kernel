#include "planificacion.h"



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Globales

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

u_int32_t grado_multiprogramacion = 0;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Colas Y listas de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_exit;
t_queue* cola_blocked;
t_queue* cola_executing;

t_list* lista_io;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                            Semaforos

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

pthread_mutex_t mutex_io = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_exit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cola_executing = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pid_counter = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_grado_multiprogramacion = PTHREAD_MUTEX_INITIALIZER;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                            Funciones de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


void inicializar_colas() {
    cola_new = queue_create();
    cola_ready = queue_create();
    cola_exit = queue_create();
    cola_blocked = queue_create();
    cola_executing = queue_create();
    lista_io = list_create();
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
    while(!queue_is_empty(cola_executing)) {
        t_pcb* pcb = peek_cola_executing();
        if(pcb != NULL && pcb->estado == EXIT) {
            t_pcb* pcb_exit = obtener_pcb_de_cola_executing();
            agregar_pcb_a_cola_exit(pcb_exit);
        }
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