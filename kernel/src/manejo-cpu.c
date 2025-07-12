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
        enviar_mensaje("Handshake recibido", conexion_dispatch);
        guardar_cpu_dispatch((void*)(intptr_t)socket_cpu_dispatch, id_cpu);
    }
    return NULL;
};

void* receptor_interrupt(void* socket_cpu_interrupt){

    int socket_interrupt = (intptr_t)socket_cpu_interrupt;
    
    while(1){
        int conexion_interrupt = esperar_cliente(socket_interrupt);
        int id_cpu = 0;
        recibir_handshake_cpu(conexion_interrupt, &id_cpu);
        enviar_mensaje("Handshake recibido", conexion_interrupt);
        guardar_cpu_interrupt((void*)(intptr_t)socket_cpu_interrupt, id_cpu);
    }
    return NULL;
};

void* guardar_cpu_dispatch(void* socket_cpu_dispatch, int id_cpu) {
    //Primero busco si existe un CPU con este id en la lista, si no existe, creo uno y lo agrego. 

    t_cpu_en_kernel* cpu_en_kernel = buscar_cpu_por_id(id_cpu);
    //Si no existe, creo uno, lo agrego a la lista y actualizo el socket de dispatch
    if(cpu_en_kernel == NULL) {
        t_cpu_en_kernel* cpu_en_kernel = malloc(sizeof(t_cpu_en_kernel));
        cpu_en_kernel->id_cpu = id_cpu;
        cpu_en_kernel->socket_cpu_dispatch = (intptr_t)socket_cpu_dispatch;
        cpu_en_kernel->esta_ocupada = false;
        list_add(lista_cpu, cpu_en_kernel);
    }
    else {
        //Si existe, actualizo el socket de dispatch
        cpu_en_kernel->socket_cpu_dispatch = (intptr_t)socket_cpu_dispatch;
    }

    //Creo el hilo que estara operando infinitamente sobre ese socket de dispatch. 
    pthread_t thread_dispatch;
    pthread_create(&thread_dispatch, NULL, manejo_dispatch, (void*)(intptr_t)socket_cpu_dispatch);
    pthread_detach(thread_dispatch);
    return NULL;
};


void* guardar_cpu_interrupt(void* socket_cpu_interrupt, int id_cpu) {

    t_cpu_en_kernel* cpu_en_kernel = buscar_cpu_por_id(id_cpu);
    if(cpu_en_kernel == NULL) {
        t_cpu_en_kernel* cpu_en_kernel = malloc(sizeof(t_cpu_en_kernel));
        cpu_en_kernel->id_cpu = id_cpu;
        cpu_en_kernel->socket_cpu_interrupt = (intptr_t)socket_cpu_interrupt;
        cpu_en_kernel->esta_ocupada = false;
        list_add(lista_cpu, cpu_en_kernel);
    }
    else {
        cpu_en_kernel->socket_cpu_interrupt = (intptr_t)socket_cpu_interrupt;
    }

    //Creo el hilo que estara operando infinitamente sobre ese socket de interrupt. 

    pthread_t thread_interrupt;
    pthread_create(&thread_interrupt, NULL, manejo_interrupt, (void*)(intptr_t)socket_cpu_interrupt);
    pthread_detach(thread_interrupt);
    return NULL;
};


void* manejo_dispatch(void* socket_cpu_dispatch){
    int socket_dispatch = (intptr_t)socket_cpu_dispatch;
    
    
    while(1){
        //Me quedo recibiendo respuestas.
    }
    return NULL;
};

void* manejo_interrupt(void* socket_cpu_interrupt){
    int socket_interrupt = (intptr_t)socket_cpu_interrupt;
    
    while(1){
        //Me quedo recibiendo respuestas.
    }
    return NULL;
};