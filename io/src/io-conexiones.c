#include "io-conexiones.h"


int io_conectar_a_kernel(){
    t_log* io_logger = iniciar_logger("io.log", "[IO]");
    int socket_kernel = crear_conexion (IP, PUERTO_KERNEL_IO, io_logger);

    log_info(io_logger, "FD de conexion con IO %d", socket_kernel);
    log_info(io_logger, "Enviando handhsake a IO...");

    if(enviar_handshake (socket_kernel, HANDSHAKE_IO, io_logger) == -1){
        log_error(io_logger, "Fallo al enviar handshake a KERNEL. Cierro conexion.");
        close(socket_kernel);
        return -1;
    }

    log_info(io_logger, "Handshake exitoso! Envio mensaje kernel '");
    enviar_mensaje("Hola kernel soy el modulo IO", socket_kernel, io_logger);
    log_info(io_logger, "Esperando respuesta del KERNEL...");

    char* respuesta = recibir_mensaje(socket_kernel, io_logger);

    log_info(io_logger, "Me llego esto:  %s", respuesta);

    
    free(respuesta);
    log_destroy(io_logger);
    close(socket_kernel);

    return 0;
}