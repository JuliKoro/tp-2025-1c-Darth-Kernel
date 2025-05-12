#include "cpu-conexiones.h"



int cpu_conectar_a_kernel(void){

    iniciar_logger_global(&logger_sockets, "cpu-conexiones.log", "[CPU-CLIENTE]");

    int socket_kernel = crear_conexion (cpu_configs.ipkernel, int_a_string(cpu_configs.puertokerneldispatch));

    log_info(logger_sockets, "FD de conexion con el KERNEL %d", socket_kernel);
    log_info(logger_sockets, "[HANDSHAKE] Enviando handhsake a KERNEL...");

    if(enviar_handshake (socket_kernel, HANDSHAKE_CPU) == -1){
        log_error(logger_sockets, "[HANDSHAKE] Fallo al enviar handshake a KERNEL. Cierro conexion.");
        close(socket_kernel);
        return -1;
    }


    
    log_info(logger_sockets, "[HANDSHAKE] Handshake exitoso! Conexion abierta con KERNEL");
    return socket_kernel;
}

 int cpu_conectar_a_memoria(void){

    iniciar_logger_global(&logger_sockets, "cpu-conexiones.log", "[CPU-CLIENTE]");

    int socket_memoria = crear_conexion (cpu_configs.ipmemoria, int_a_string(cpu_configs.puertomemoria));

    log_info(logger_sockets, "FD de conexion con la MEMORIA %d", socket_memoria);
    log_info(logger_sockets, "[HANDSHAKE] Enviando handhsake a MEMORIA...");

    if(enviar_handshake (socket_memoria, HANDSHAKE_CPU) == -1) {
        log_error(logger_sockets, "[HANDSHAKE] Fallo al enviar handshake a MEMORIA. Cierro conexion.");
        close(socket_memoria);
        return -1;
    }

    log_info(logger_sockets, "[HANDSHAKE] Handshake exitoso! Conexion abierta con MEMORIA");
    return socket_memoria;

}