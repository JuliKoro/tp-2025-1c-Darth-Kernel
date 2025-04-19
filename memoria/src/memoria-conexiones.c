#include "memoria-conexiones.h"

int memoria_conectar_a_kernel(){
    id_modulo_t modulo_recibido;
    t_log* memoria_logger =  iniciar_logger("memoria.log", "[MEMORIA]");
    int memoria_server_fd = iniciar_servidor (PUERTO_MEMORIA_KERNEL, memoria_logger);
    int memoria_cliente_fd = esperar_cliente(memoria_server_fd, memoria_logger);

    log_info(memoria_logger, "FD del KERNEL %d", memoria_cliente_fd);
    log_info(memoria_logger, "Esperando handshake de KERNEL..");
    
    // Chequeo recepcion y tipo de handshake
    if(recibir_handshake(memoria_cliente_fd, &modulo_recibido, memoria_logger) == -1 ) {
        log_error(memoria_logger, "Error en recepcion de handshake. Cierro conexion. ");
        close(memoria_cliente_fd);
        return -1;
    }
    if (modulo_recibido != HANDSHAKE_KERNEL){
        log_error(memoria_logger, "Se esperaba conexion de KERNEL. Cierro conexion.");
        close(memoria_cliente_fd);
        return -1;
    }
    
    // Una vez que pasa el handshake y el socket sigue abierto sigo

    log_info(memoria_logger, "Handshake exitoso! Espero mensaje de KERNEL");

    char* mensaje = recibir_mensaje(memoria_cliente_fd, memoria_logger);

    log_info(memoria_logger, "Me llego esto: %s", mensaje);

    log_info(memoria_logger, "Respondiendo a KERNEL...");

    enviar_mensaje("Hola KERNEL tanto tiempo", memoria_cliente_fd, memoria_logger);


    free(mensaje);
    close(memoria_cliente_fd);
    close(memoria_server_fd);
    log_destroy(memoria_logger);

    return 0;

}




int memoria_conectar_a_cpu(){
    id_modulo_t modulo_recibido;
    t_log* memoria_logger =  iniciar_logger("memoria.log", "[MEMORIA]");
    int memoria_server_fd = iniciar_servidor (PUERTO_MEMORIA_CPU, memoria_logger);
    int memoria_cliente_fd = esperar_cliente(memoria_server_fd, memoria_logger);

    log_info(memoria_logger, "FD del CPU %d", memoria_cliente_fd);
    log_info(memoria_logger, "Esperando handshake de CPU..");
    
    // Chequeo recepcion y tipo de handshake
    if(recibir_handshake(memoria_cliente_fd, &modulo_recibido, memoria_logger) == -1 ) {
        log_error(memoria_logger, "Error en recepcion de handshake. Cierro conexion. ");
        close(memoria_cliente_fd);
        return -1;
    }
    if (modulo_recibido != HANDSHAKE_CPU){
        log_error(memoria_logger, "Se esperaba conexion de CPU. Cierro conexion.");
        close(memoria_cliente_fd);
        return -1;
    }
    
    // Una vez que pasa el handshake y el socket sigue abierto sigo

    log_info(memoria_logger, "Handshake exitoso! Espero mensaje de CPU");

    char* mensaje = recibir_mensaje(memoria_cliente_fd, memoria_logger);

    log_info(memoria_logger, "Me llego esto: %s", mensaje);

    log_info(memoria_logger, "Respondiendo a CPU...");

    enviar_mensaje("Hola CPU!", memoria_cliente_fd, memoria_logger);


    free(mensaje);
    close(memoria_cliente_fd);
    close(memoria_server_fd);
    log_destroy(memoria_logger);

    return 0;

}