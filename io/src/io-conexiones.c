#include "io-conexiones.h"


int io_conectar_a_kernel(){
    iniciar_logger_global(&logger_sockets, "io-conexiones.log", "[IO-CLIENTE]");
    int socket_kernel = crear_conexion (io_configs.ipkernel, int_a_string(io_configs.puertokernel));

    log_info(logger_sockets, "FD de conexion con Kernel %d", socket_kernel);
    log_info(logger_sockets, "Enviando handhsake a Kernel...");

    if(enviar_handshake (socket_kernel, HANDSHAKE_IO) == -1){
        log_error(logger_sockets, "Fallo al enviar handshake a KERNEL. Cierro conexion.");
        close(socket_kernel);
        return -1;
    }


    log_info(logger_sockets, "Handshake exitoso! Conexion abierta con KERNEL");

    
    return socket_kernel;
}