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
    pthread_create(&thread_dispatch, NULL, receptor_dispatch, (void*)socket_cpu_dispatch);
    pthread_detach(thread_dispatch);

    //Creo un hilo para el receptor de interrupt, va a estar esperando conexiones
    pthread_t thread_interrupt;
    pthread_create(&thread_interrupt, NULL, receptor_interrupt, (void*)socket_cpu_interrupt);
    pthread_detach(thread_interrupt);
}

void* receptor_dispatch(void* socket_cpu_dispatch){
    int socket_dispatch = (int)socket_cpu_dispatch;

    
    while(1){
        int conexion_dispatch = esperar_cliente(socket_dispatch);
        int id_cpu = 0;
        recibir_handshake_cpu(conexion_dispatch, &id_cpu);
        guardar_cpu_dispatch(conexion_dispatch, id_cpu);
    }
};

void* receptor_interrupt(void* socket_cpu_interrupt){};

void* guardar_cpu_dispatch(void* socket_cpu_dispatch, int id_cpu) {
    //Primero busco si existe un CPU con este id en la lista, si no existe, creo uno y lo agrego. TODO
    t_cpu_en_kernel* cpu_en_kernel = malloc(sizeof(t_cpu_en_kernel));
    cpu_en_kernel->id_cpu = id_cpu;
    cpu_en_kernel->socket_cpu_dispatch = (int)socket_cpu_dispatch;
    list_add(lista_cpu, cpu_en_kernel);
    //Si existe, actualizo el socket de dispatch

    //Creo el hilo que estara operando infinitamente sobre ese socket de dispatch
};

void* guardar_cpu_interrupt(void* socket_cpu_interrupt, int id_cpu) {};