#include "kernel-conexiones.h"

int iniciar_servidor_generico(int puerto, char* nombre_modulo, int tipo_modulo) {

    id_modulo_t modulo_recibido;

    iniciar_logger_global(&logger_sockets, "kernel-conexiones.log", "[KERNEL-SERVIDOR]");

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

    iniciar_logger_global(&logger_sockets, "kernel-conexiones.log", "[KERNEL-MEMORIA]");

    int socket_memoria = crear_conexion(kernel_configs.ipmemoria, int_a_string(kernel_configs.puertomemoria));

    log_info(logger_sockets, "FD de conexion con la MEMORIA %d", socket_memoria);
    log_info(logger_sockets, "[HANDSHAKE] Enviando handhsake a MEMORIA...");

    if(enviar_handshake (socket_memoria, HANDSHAKE_KERNEL) == -1) {
        log_error(logger_sockets, "[HANDSHAKE] Fallo al enviar handshake a MEMORIA. Cierro conexion.");
        close(socket_memoria);
        return -1;
    }

    log_info(logger_sockets, "[HANDSHAKE] Handshake exitoso! Conexion abierta con MEMORIA");
    return socket_memoria;
}