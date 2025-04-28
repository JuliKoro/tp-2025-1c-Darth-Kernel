#include "io-conexiones.h"


int io_conectar_a_kernel(){
    t_log* io_logger_conexiones = iniciar_logger("io.log", "[IO]");
    logger_sockets= io_logger_conexiones;
    int socket_kernel = crear_conexion (IP, PUERTO_KERNEL_IO);

    log_info(io_logger_conexiones, "FD de conexion con Kernel %d", socket_kernel);
    log_info(io_logger_conexiones, "Enviando handhsake a Kernel...");

    if(enviar_handshake (socket_kernel, HANDSHAKE_IO) == -1){
        log_error(io_logger_conexiones, "Fallo al enviar handshake a KERNEL. Cierro conexion.");
        close(socket_kernel);
        return -1;
    }

    log_info(io_logger_conexiones, "Handshake exitoso! Envio mensaje kernel '");
    enviar_mensaje("Hola kernel soy el modulo IO", socket_kernel);
    log_info(io_logger_conexiones, "Esperando respuesta del KERNEL...");

    char* respuesta = recibir_mensaje(socket_kernel);

    log_info(io_logger_conexiones, "Me llego esto:  %s", respuesta);

    
    free(respuesta);
    log_destroy(io_logger_conexiones);
    close(socket_kernel);

    return 0;
}