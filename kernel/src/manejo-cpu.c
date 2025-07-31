#include "manejo-cpu.h"

//orden en cpu, idealmente

//int socket_cpu_dispatch = crear_conexion (ipkernel, puerto dispatch);
//int socket_cpu_interrupt = crear_conexion (ipkernel, puerto interrupt);

//En cpu obtengo los sockets, envio primero handshake por dispatch
//Kernel recibe, crea el t_cpu_en_kernel y lo agrega a la lista de cpus con su socket de dispatch

//En cpu, mando el handshake por interrupt
//Kernel recibe, con el id de la cpu, lo busca en la lista de cpus y guarda el socket de interrupt

//Dejo abiertos los sockets de dispatch y interrupt, con la logica que corresponda en cada caso



void* iniciar_receptores_cpu() {
    int socket_cpu_dispatch = iniciar_servidor_dispatch();
    int socket_cpu_interrupt = iniciar_servidor_interrupt();

    //Creo un hilo para el receptor de dispatch, va a estar esperando conexiones
    pthread_t thread_dispatch;
    pthread_create(&thread_dispatch, NULL, receptor_dispatch, (void*)(intptr_t)socket_cpu_dispatch);
    pthread_detach(thread_dispatch);

    //Creo un hilo para el receptor de interrupt, va a estar esperando conexiones
    pthread_t thread_interrupt;
    pthread_create(&thread_interrupt, NULL, receptor_interrupt, (void*)(intptr_t)socket_cpu_interrupt);
    pthread_detach(thread_interrupt);
    return NULL;
}

void* receptor_dispatch(void* socket_cpu_dispatch){
    int socket_dispatch = (intptr_t)socket_cpu_dispatch;
    
    while(1){
        int conexion_dispatch = esperar_cliente(socket_dispatch);
        int id_cpu = 0;
        recibir_handshake_cpu(conexion_dispatch, &id_cpu);
        log_info(logger_kernel, "Se estableció conexión con CPU ID: %d en el canal Dispatch", id_cpu);
        enviar_mensaje("Handshake recibido", conexion_dispatch);
        guardar_cpu_dispatch((void*)(intptr_t)conexion_dispatch, id_cpu);
    }
    return NULL;
};

void* receptor_interrupt(void* socket_cpu_interrupt){

    int socket_interrupt = (intptr_t)socket_cpu_interrupt;
    
    while(1){
        int conexion_interrupt = esperar_cliente(socket_interrupt);
        int id_cpu = 0;
        recibir_handshake_cpu(conexion_interrupt, &id_cpu);
        log_info(logger_kernel, "Se estableció conexión con CPU ID: %d en el canal Interrupt", id_cpu);
        enviar_mensaje("Handshake recibido", conexion_interrupt);
        guardar_cpu_interrupt((void*)(intptr_t)conexion_interrupt, id_cpu);
    }
    return NULL;
};

void* guardar_cpu_dispatch(void* socket_cpu_dispatch, int id_cpu) {
    //Primero busco si existe un CPU con este id en la lista, si no existe, creo uno y lo agrego. 
    pthread_mutex_lock(&mutex_cpu);
    t_cpu_en_kernel* cpu_en_kernel = buscar_cpu_por_id_unsafe(id_cpu);
    //Si no existe, creo uno, lo agrego a la lista y actualizo el socket de dispatch
    if(cpu_en_kernel == NULL) {
        t_cpu_en_kernel* cpu_nueva = malloc(sizeof(t_cpu_en_kernel));
        cpu_nueva->id_cpu = id_cpu;
        cpu_nueva->socket_cpu_dispatch = (intptr_t)socket_cpu_dispatch;
        cpu_nueva->esta_ocupada = false;
        cpu_nueva->pid_actual = -1;
        list_add(lista_cpu, cpu_nueva);
        log_debug(logger_kernel, "[CPU Management] Nueva CPU (ID: %d, Dispatch Socket: %d) creada y añadida a la lista. Dirección: %p", cpu_nueva->id_cpu, cpu_nueva->socket_cpu_dispatch, (void*)cpu_nueva); // DEBUG_LOG
        pthread_mutex_unlock(&mutex_cpu);
        sem_post(&sem_cpu_disponible);
    }
    else {
        //Si existe, actualizo el socket de dispatch
        cpu_en_kernel->socket_cpu_dispatch = (intptr_t)socket_cpu_dispatch;
        pthread_mutex_unlock(&mutex_cpu);
        sem_post(&sem_cpu_disponible);
    }

    //Creo el hilo que estara operando infinitamente sobre ese socket de dispatch. 
    pthread_t thread_dispatch;
    pthread_create(&thread_dispatch, NULL, manejo_dispatch, (void*)(intptr_t)socket_cpu_dispatch);
    pthread_detach(thread_dispatch);
    return NULL;
};


void* guardar_cpu_interrupt(void* socket_cpu_interrupt, int id_cpu) {

    pthread_mutex_lock(&mutex_cpu);
    t_cpu_en_kernel* cpu_en_kernel = buscar_cpu_por_id_unsafe(id_cpu);
    if(cpu_en_kernel == NULL) {
        t_cpu_en_kernel* cpu_nueva = malloc(sizeof(t_cpu_en_kernel));
        cpu_nueva->id_cpu = id_cpu;
        cpu_nueva->socket_cpu_interrupt = (intptr_t)socket_cpu_interrupt;
        cpu_nueva->esta_ocupada = false;
        cpu_nueva->pid_actual = -1;
        list_add(lista_cpu, cpu_nueva);
        log_debug(logger_kernel, "[CPU Management] Nueva CPU (ID: %d, Interrupt Socket: %d) creada y añadida a la lista. Dirección: %p", cpu_nueva->id_cpu, cpu_nueva->socket_cpu_interrupt, (void*)cpu_nueva); // DEBUG_LOG
        pthread_mutex_unlock(&mutex_cpu);
        sem_post(&sem_cpu_disponible);
    }
    else {
        cpu_en_kernel->socket_cpu_interrupt = (intptr_t)socket_cpu_interrupt;
        pthread_mutex_unlock(&mutex_cpu);
        sem_post(&sem_cpu_disponible);
    }

    //Creo el hilo que estara operando infinitamente sobre ese socket de interrupt. 

    pthread_t thread_interrupt;
    pthread_create(&thread_interrupt, NULL, manejo_interrupt, (void*)(intptr_t)socket_cpu_interrupt);
    pthread_detach(thread_interrupt);
    return NULL;
};


void* manejo_dispatch(void* socket_cpu_dispatch){
    int socket_dispatch = (intptr_t)socket_cpu_dispatch;
    t_paquete* paquete = NULL;
    
    while(1){
        //Me quedo recibiendo respuestas.
        
        
        //Detecto desconexiones
        paquete = recibir_paquete(socket_dispatch);

        if(paquete == NULL) {
            log_info(logger_kernel, "CPU desconectada, eliminando de la lista");
            eliminar_cpu_por_socket(socket_dispatch);
            break;
        }

        if(paquete->codigo_operacion == PAQUETE_SYSCALL) {
            t_syscall* syscall = deserializar_syscall(paquete->buffer);
            manejar_syscall(syscall);
            free(syscall->syscall);
            free(syscall);
        }
        
        if(paquete->codigo_operacion == PAQUETE_INTERRUPCION) {
            t_interrupcion* interrupcion = deserializar_interrupcion(paquete->buffer);
            actualizar_pcb(interrupcion->pid, interrupcion->pc);
            free(interrupcion);
        }

    }
    return NULL;
};

void* manejo_interrupt(void* socket_cpu_interrupt){
    int socket_interrupt = (intptr_t)socket_cpu_interrupt;
    t_paquete* paquete = NULL;
    while(1){
        //Me quedo recibiendo respuestas.
        paquete = recibir_paquete(socket_interrupt);

        if(paquete == NULL) {
            //Ya fue eliminado en el hilo de manejo_dispatch
            break;
        }
        
        liberar_paquete(paquete);
    }
    return NULL;
};


void* eliminar_cpu_por_socket(int socket) {
    pthread_mutex_lock(&mutex_cpu);
    int indice = buscar_cpu_por_socket_unsafe(socket);
    if(indice != -1) {
        t_cpu_en_kernel* cpu_en_kernel = list_remove(lista_cpu, indice);
        pthread_mutex_unlock(&mutex_cpu);
        log_info(logger_kernel, "CPU %d eliminada de la lista", cpu_en_kernel->id_cpu); 

        //Antes de eliminar la cpu por completo, me fijo si estaba ejecutando algun proceso
        //Si lo estaba, lo muevo a la cola de ready
        if(cpu_en_kernel->esta_ocupada) {
            mover_executing_a_ready(cpu_en_kernel->pid_actual);
        }


        close(cpu_en_kernel->socket_cpu_dispatch);
        close(cpu_en_kernel->socket_cpu_interrupt);
        log_debug(logger_kernel, "[CPU Management] Liberando memoria para CPU ID %d. Dirección: %p", cpu_en_kernel->id_cpu, (void*)cpu_en_kernel); // DEBUG_LOG
        free(cpu_en_kernel);
        sem_trywait(&sem_cpu_disponible);
    } else {
        pthread_mutex_unlock(&mutex_cpu);
    }
    
    return NULL;
}