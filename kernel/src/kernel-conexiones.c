#include "kernel-conexiones.h"





int kernel_conectar_a_cpu(){

    id_modulo_t modulo_recibido;
    t_log* kernel_logger =  iniciar_logger("kernel.log", "[KERNEL-S]");
    int kernel_server_fd = iniciar_servidor (PUERTO_KERNEL_CPU, kernel_logger);
    int kernel_cliente_fd = esperar_cliente(kernel_server_fd, kernel_logger);

    log_info(kernel_logger, "FD del CPU conectado %d", kernel_cliente_fd);
    log_info(kernel_logger, "Esperando handshake de CPU..");
    
    // Chequeo recepcion y tipo de handshake
    if(recibir_handshake(kernel_cliente_fd, &modulo_recibido, kernel_logger) == -1 ) {
        log_error(kernel_logger, "Error en recepcion de handshake. Cierro conexion. ");
        close(kernel_cliente_fd);
        return -1;
    }
    if (modulo_recibido != HANDSHAKE_CPU){
        log_error(kernel_logger, "Se esperaba conexion de CPU. Cierro conexion.");
        close(kernel_cliente_fd);
        return -1;
    }
    
    // Una vez que pasa el handshake y el socket sigue abierto sigo

    log_info(kernel_logger, "Handshake exitoso! Espero mensaje de CPU");

    char* mensaje = recibir_mensaje(kernel_cliente_fd, kernel_logger);

    log_info(kernel_logger, "Me llego esto: %s", mensaje);

    log_info(kernel_logger, "Respondiendo a CPU...");

    enviar_mensaje("Hola CPU strong handshake! ;)", kernel_cliente_fd, kernel_logger);


    free(mensaje);
    close(kernel_cliente_fd);
    close(kernel_server_fd);
    log_destroy(kernel_logger);

    return 0;
}
int kernel_conectar_a_io(){
    id_modulo_t modulo_recibido;
    t_log* kernel_logger =  iniciar_logger("kernel.log", "[KERNEL-S]");
    int kernel_server_fd = iniciar_servidor (PUERTO_KERNEL_IO, kernel_logger);
    int kernel_cliente_fd = esperar_cliente(kernel_server_fd, kernel_logger);

    log_info(kernel_logger, "FD del IO conectado %d", kernel_cliente_fd);
    log_info(kernel_logger, "Esperando handshake de IO..");
    
     // Chequeo recepcion y tipo de handshake
     if(recibir_handshake(kernel_cliente_fd, &modulo_recibido, kernel_logger) == -1 ) {
        log_error(kernel_logger, "Error en recepcion de handshake. Cierro conexion. ");
        close(kernel_cliente_fd);
        return -1;
    }
    if (modulo_recibido != HANDSHAKE_IO){
        log_error(kernel_logger, "Se esperaba conexion de IO. Cierro conexion.");
        close(kernel_cliente_fd);
        return -1;
    }
    
    // Una vez que pasa el handshake y el socket sigue abierto sigo

    log_info(kernel_logger, "Handshake exitoso! Espero mensaje de IO");

    char* mensaje = recibir_mensaje(kernel_cliente_fd, kernel_logger);

    log_info(kernel_logger, "Me llego esto: %s", mensaje);

    log_info(kernel_logger, "Respondiendo a IO...");

    enviar_mensaje("Hola IO!", kernel_cliente_fd, kernel_logger);


    free(mensaje);
    close(kernel_cliente_fd);
    close(kernel_server_fd);
    log_destroy(kernel_logger);

    return 0;
}
int kernel_conectar_a_memoria(){
    t_log* io_logger = iniciar_logger("kernel.log", "[KERNEL-C]");
    int socket_memoria = crear_conexion (IP, PUERTO_MEMORIA_KERNEL, io_logger);

    log_info(io_logger, "FD de conexion con MEMORIA %d", socket_memoria);
    log_info(io_logger, "Enviando handhsake a MEMORIA...");

    if(enviar_handshake (socket_memoria, HANDSHAKE_KERNEL, io_logger) == -1){
        log_error(io_logger, "Fallo al enviar handshake a MEMORIA. Cierro conexion.");
        close(socket_memoria);
        return -1;
    }

    log_info(io_logger, "Handshake exitoso! Envio mensaje kernel '");
    enviar_mensaje("Hola memoria soy el modulo KERNEL", socket_memoria, io_logger);
    log_info(io_logger, "Esperando respuesta de la MEMORIA...");

    char* respuesta = recibir_mensaje(socket_memoria, io_logger);

    log_info(io_logger, "Me llego esto:  %s", respuesta);

    
    free(respuesta);
    log_destroy(io_logger);
    close(socket_memoria);

    return 0;
}