#include "memoria-conexiones.h"
#include "memoria-procesos.h"



int iniciar_servidor_memoria(){

    id_modulo_t modulo_recibido;
    iniciar_logger_global(&logger_sockets, "memoria-conexiones.log", "[MEMORIA-SERVIDOR]");

    //Creo el socket del servidor
    int memoria_server_fd = iniciar_servidor(int_a_string(memoria_configs.puertoescucha));

    return memoria_server_fd;
}

void atender_peticion_kernel(void* socket_cliente){
    int socket_fd = *(int*) socket_cliente;
    free(socket_cliente);

    id_modulo_t modulo_recibido;

    if(recibir_handshake(socket_fd, &modulo_recibido) == -1){
        log_error(logger_sockets, "[HANDSHAKE] Error en recepcion de handshake. Cierro conexion.");
        close(socket_fd);
        return;
    }

    log_info(logger_memoria, "[HANDSHAKE] Handshake recibido correctamente. Conexion establecida con kernel");

    
        //Recibir pid, tamanio, pc, archivo pseudocodigo
        t_paquete* paquete = recibir_paquete(socket_fd);
        t_pcb* pcb = deserializar_pcb(paquete->buffer);
        liberar_paquete(paquete);

        //Ver si puedo cargar instrucciones en memoria
        if(cargar_proceso(pcb->pid, pcb->archivo_pseudocodigo) == -1){
            log_error(logger_memoria, "[CARGAR PROCESO] Error al cargar proceso. PID: %d", pcb->pid);
            enviar_bool(socket_fd, false);
        }

    //Enviar respuesta al kernel
    enviar_bool(socket_fd, true);      
    
    close(socket_fd);
}

 void* recibir_peticiones_kernel(void* socket_memoria){
    int socket_fd = *(int*) socket_memoria;
    //free(socket_memoria);
    while(true){
        int cliente_fd = esperar_cliente(socket_fd);

        pthread_t hilo_cliente;
        int* socket_cliente = malloc(sizeof(int));
        *socket_cliente = cliente_fd;

        pthread_create(&hilo_cliente, NULL, (void*)atender_peticion_kernel, (void*) socket_cliente);
        pthread_detach(hilo_cliente);
    }
}


