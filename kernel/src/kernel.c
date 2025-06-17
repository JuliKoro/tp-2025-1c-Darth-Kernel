#include "kernel.h"

void enviar_solicitud_io(char* nombre_io, uint32_t pid, uint32_t tiempo);

int main(int argc, char* argv[]) {

    // //Chequeo que los parametros sean correctos
    // if(argc != 3) {
    //     fprintf(stderr, "Uso: %s [archivo_pseudocodigo] [tamanio_proceso]\n", argv[0]); 
    //     return EXIT_FAILURE;
    // }

    // //Obtengo los parametros
    // int tamanio_proceso = atoi(argv[2]);
    // char* archivo_pseudocodigo = argv[1];

    //Primero levanto las configs, despues el logger
    lista_io = list_create();
    inicializar_colas();
    inicializar_configs();
    inicializar_logger_kernel();

    //planificar_proceso_inicial(archivo_pseudocodigo, tamanio_proceso);
    
    //Inicia el receptor de IO
    pthread_t thread_receptor_io;
    pthread_create(&thread_receptor_io, NULL, iniciar_receptor_io, NULL);
    pthread_detach(thread_receptor_io); 


    sleep(5);
    enviar_solicitud_io("impresora", 1, 5);

    // while(1) {
    //     sleep(20);
    //     mostrar_lista_io();
    // }
   


    destruir_logger_kernel();
    config_destroy(kernel_tconfig);
    return 0;
}

void enviar_solicitud_io(char* nombre_io, uint32_t pid, uint32_t tiempo){

    pthread_mutex_lock(&mutex_io);
    
    // Busco el IO en la lista
    t_io* io = list_find_con_param(lista_io, comparar_nombre_io, nombre_io);
    
    if(io == NULL) {
        log_error(logger_kernel, "[IO-SOLICITUD] No se encontró el IO %s", nombre_io);
        pthread_mutex_unlock(&mutex_io);
        return;
    }
    
    // Busco una instancia disponible (pid_actual = -1)
    t_instancia_io* instancia_disponible = NULL;
    for(int i = 0; i < list_size(io->instancias_io); i++) {
        t_instancia_io* instancia = list_get(io->instancias_io, i);
        if(instancia->pid_actual == -1) {
            instancia_disponible = instancia;
            break;
        }
    }
    
    if(instancia_disponible == NULL) {
        log_error(logger_kernel, "[IO-SOLICITUD] No hay instancias disponibles para %s", nombre_io);
        pthread_mutex_unlock(&mutex_io);
        return;
    }
    
    // Marco la instancia como ocupada
    instancia_disponible->pid_actual = pid;
    io->instancias_disponibles--;
    
    log_info(logger_kernel, "[IO-SOLICITUD] Enviando solicitud a %s (PID: %d, tiempo: %d)", nombre_io, pid, tiempo);
    
    // Creo la solicitud
    t_solicitud_io solicitud;
    solicitud.pid = pid;
    solicitud.tiempo = tiempo;
    
    // Serializo y envío
    t_buffer* buffer = serializar_solicitud_io(&solicitud);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_SOLICITUD_IO, buffer);
    
    enviar_paquete(instancia_disponible->socket_io, paquete);

    char* mensaje = recibir_mensaje(instancia_disponible->socket_io);
    log_info(logger_kernel, "[IO-SOLICITUD] Mensaje recibido: %s", mensaje);
    free(mensaje);
    
    pthread_mutex_unlock(&mutex_io);
}
