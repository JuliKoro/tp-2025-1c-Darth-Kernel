#include "kernel-conexiones.h"

int iniciar_servidor_generico(int puerto, char* nombre_modulo, int tipo_modulo) {


    //Creo el socket del servidor
    int kernel_server_fd = iniciar_servidor(int_a_string(puerto));
    //Espero la conexion
    //int kernel_cliente_fd = esperar_cliente(kernel_server_fd);

    //Realizo el handshake
    

    return kernel_server_fd;
}

int iniciar_servidor_dispatch() {
    return iniciar_servidor_generico(kernel_configs.escuchadispatch, "CPU", HANDSHAKE_CPU);
}

int iniciar_servidor_io() {
    return iniciar_servidor_generico(kernel_configs.escuchaio, "IO", HANDSHAKE_IO);
}

int iniciar_servidor_interrupt() {
    return iniciar_servidor_generico(kernel_configs.escuchainterrupt, "CPU-INTERRUPT", HANDSHAKE_CPU);
}

int kernel_conectar_a_memoria(){
    //Obtengo ip y puerto de memoria
    char* ip_memoria = kernel_configs.ipmemoria;
    char* puerto_memoria = kernel_configs.puertomemoria;

    int socket_memoria = crear_conexion(ip_memoria, puerto_memoria);
    
    //log_info(logger_sockets, "FD de conexion con la MEMORIA %d", socket_memoria);

    return socket_memoria;
}

bool solicitar_creacion_proceso(t_pcb* pcb) {
    int socket_memoria;
    bool respuesta;
    socket_memoria = kernel_conectar_a_memoria();
    if(socket_memoria == -1) {
        log_error(logger_kernel, "Error al solicitar creacion de un proceso a memoria, conexion con memoria fallida");
        return false;
    }
    
    t_paquete* paquete = empaquetar_buffer(PAQUETE_CARGAR_PROCESO, serializar_pcb(pcb));
    enviar_paquete(socket_memoria, paquete);
    respuesta = recibir_bool(socket_memoria);
    close(socket_memoria);
    return respuesta;
}