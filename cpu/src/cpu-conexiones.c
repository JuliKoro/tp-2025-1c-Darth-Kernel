#include "cpu-conexiones.h"
#include <utils/sockets.h>


int cpu_conectar_a_kernel(void){

    t_log* cpu_logger_conexiones = iniciar_logger("cpu.log", "[CPU]");
    logger_sockets = cpu_logger_conexiones;
    int socket_kernel = crear_conexion (IP, PUERTO_KERNEL_CPU);

    log_info(logger_sockets, "FD de conexion con el KERNEL %d", socket_kernel);
    log_info(logger_sockets, "[HANDSHAKE] Enviando handhsake a KERNEL...");

    if(enviar_handshake (socket_kernel, HANDSHAKE_CPU) == -1){
        log_error(logger_sockets, "[HANDSHAKE] Fallo al enviar handshake a KERNEL. Cierro conexion.");
        close(socket_kernel);
        return -1;
    }

    log_info(logger_sockets, "[HANDSHAKE] Handshake exitoso! Envio mensaje a KERNEL");
    enviar_mensaje("Hola kernel como andas :3 soy el CPU", socket_kernel);
    log_info(logger_sockets, "Esperando respuesta del KERNEL...");

    char* respuesta = recibir_mensaje(socket_kernel);

    log_info(logger_sockets, "Me llego esto:  %s", respuesta);

    
    free(respuesta);
    log_destroy(logger_sockets);
    close(socket_kernel);


    return 0;


}

// int cpu_conectar_a_memoria(void){
//     t_log* cpu_logger = iniciar_logger("cpu.log", "[CPU]");
//     int socket_memoria = crear_conexion (IP, PUERTO_MEMORIA_CPU, cpu_logger);

//     log_info(cpu_logger, "FD de conexion con la MEMORIA %d", socket_memoria);
//     log_info(cpu_logger, "Enviando handhsake a MEMORIA...");

//     if(enviar_handshake (socket_memoria, HANDSHAKE_CPU, cpu_logger) == -1){
//         log_error(cpu_logger, "Fallo al enviar handshake a MEMORIA. Cierro conexion.");
//         close(socket_memoria);
//         return -1;
//     }

//     log_info(cpu_logger, "Handshake exitoso! Envio mensaje a MEMORIA '");
//     enviar_mensaje("Hola memoria como andas :D soy el CPU", socket_memoria, cpu_logger);
//     log_info(cpu_logger, "Esperando respuesta de MEMORIA...");

//     char* respuesta = recibir_mensaje(socket_memoria, cpu_logger);

//     log_info(cpu_logger, "Me llego esto:  %s", respuesta);

    
//     free(respuesta);
//     log_destroy(cpu_logger);
//     close(socket_memoria);

//     return 0;

// }