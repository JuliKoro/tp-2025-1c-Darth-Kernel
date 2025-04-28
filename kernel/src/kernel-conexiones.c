#include "kernel-conexiones.h"





int kernel_conectar_a_cpu(){

    id_modulo_t modulo_recibido;

    //Esta funcion crea un logger para kernel-conexiones, e iguala logger_sockets (el global) a ese logger para que pueda ser usado por las funciones de sockets.
    iniciar_logger_global(&logger_sockets, "kernel-conexiones.log", "[KERNEL-SERVIDOR]");

    int kernel_server_fd = iniciar_servidor (PUERTO_KERNEL_CPU);
    int kernel_cliente_fd = esperar_cliente(kernel_server_fd);

    log_info(logger_sockets, "FD del CPU conectado %d", kernel_cliente_fd);
    log_info(logger_sockets, "[HANDSHAKE] Esperando handshake de CPU..");
    
    // Chequeo recepcion y tipo de handshake
    if(recibir_handshake(kernel_cliente_fd, &modulo_recibido) == -1 ) {
        log_error(logger_sockets, "[HANDSHAKE] Error en recepcion de handshake. Cierro conexion. ");
        close(kernel_cliente_fd);  
        return -1;
    }
    if (modulo_recibido != HANDSHAKE_CPU){
        log_error(logger_sockets, "[HANDSHAKE] Se esperaba conexion de CPU. Cierro conexion.");
        close(kernel_cliente_fd);
        return -1;
    }
    
    // Una vez que pasa el handshake y el socket sigue abierto sigo

    log_info(logger_sockets, "[HANDSHAKE] Handshake exitoso! Espero mensaje de CPU");

    char* mensaje = recibir_mensaje(kernel_cliente_fd);

    log_info(logger_sockets, "Me llego esto: %s", mensaje);

    log_info(logger_sockets, "Respondiendo a CPU...");

    enviar_mensaje("Hola CPU strong handshake! ;)",kernel_cliente_fd);


    free(mensaje);
    close(kernel_cliente_fd);
    close(kernel_server_fd);



    return 0;
}
int kernel_conectar_a_io(){
    id_modulo_t modulo_recibido;

    //Esta funcion crea un logger para kernel-conexiones, e iguala logger_sockets a ese logger para que pueda ser usado por las funciones de sockets.
    iniciar_logger_global(&logger_sockets, "kernel-conexiones.log", "[KERNEL-SERVIDOR]");

    int kernel_server_fd = iniciar_servidor (PUERTO_KERNEL_IO);
    int kernel_cliente_fd = esperar_cliente(kernel_server_fd);

    log_info(logger_sockets, "FD del modulo IO conectado %d", kernel_cliente_fd);
    log_info(logger_sockets, "[HANDSHAKE] Esperando handshake de IO..");
    
    // Chequeo recepcion y tipo de handshake
    if(recibir_handshake(kernel_cliente_fd, &modulo_recibido) == -1 ) {
        log_error(logger_sockets, "[HANDSHAKE] Error en recepcion de handshake. Cierro conexion. ");
        close(kernel_cliente_fd);  
        return -1;
    }
    if (modulo_recibido != HANDSHAKE_IO){
        log_error(logger_sockets, "[HANDSHAKE] Se esperaba conexion de IO. Cierro conexion.");
        close(kernel_cliente_fd);
        return -1;
    }
    
    // Una vez que pasa el handshake y el socket sigue abierto sigo

    log_info(logger_sockets, "[HANDSHAKE] Handshake exitoso! Espero mensaje de IO");

    char* mensaje = recibir_mensaje(kernel_cliente_fd);

    log_info(logger_sockets, "Me llego esto: %s", mensaje);

    log_info(logger_sockets, "Respondiendo a IO...");

    enviar_mensaje("Hola IO como te va?",kernel_cliente_fd);


    free(mensaje);
    close(kernel_cliente_fd);
    close(kernel_server_fd);
    


    return 0;
}
int kernel_conectar_a_memoria(){
    
    //Esta funcion crea un logger para kernel-conexiones, e iguala logger_sockets a ese logger para que pueda ser usado por las funciones de sockets.
    iniciar_logger_global(&logger_sockets, "kernel-conexiones.log", "[KERNEL-CLIENTE]");

    int socket_memoria = crear_conexion (IP, PUERTO_MEMORIA_KERNEL);

    log_info(logger_sockets, "FD de conexion con MEMORIA %d", socket_memoria);
    log_info(logger_sockets, "Enviando handhsake a MEMORIA...");

    if(enviar_handshake (socket_memoria, HANDSHAKE_KERNEL) == -1){
        log_error(logger_sockets, "Fallo al enviar handshake a MEMORIA. Cierro conexion.");
        close(socket_memoria);
        return -1;
    }

    log_info(logger_sockets, "Handshake exitoso! Envio mensaje kernel '");
    enviar_mensaje("Hola memoria soy el modulo KERNEL", socket_memoria);
    log_info(logger_sockets, "Esperando respuesta de la MEMORIA...");

    char* respuesta = recibir_mensaje(socket_memoria);

    log_info(logger_sockets, "Me llego esto:  %s", respuesta);

    
    free(respuesta);
   
    close(socket_memoria);

    return 0;
}