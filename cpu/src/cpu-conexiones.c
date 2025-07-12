#include "cpu-conexiones.h"

int cpu_conectar_a_kernel(int puerto_kernel, int id_cpu){

    int socket_kernel = crear_conexion (cpu_configs.ipkernel, int_a_string(puerto_kernel));
    
    
    return socket_kernel; 
}

int cpu_conectar_a_memoria(int id_cpu){
    t_log* cpu_logger = iniciar_logger_cpu(id_cpu);

    int socket_memoria = crear_conexion (cpu_configs.ipmemoria, int_a_string(cpu_configs.puertomemoria));

    log_info(cpu_logger, "FD de conexion con la MEMORIA %d", socket_memoria);
    log_info(cpu_logger, "Enviando handhsake a MEMORIA...");

    if(enviar_handshake_cpu(socket_memoria, id_cpu) == -1){
        log_error(cpu_logger, "Fallo al enviar handshake a MEMORIA. Cierro conexion.");
        close(socket_memoria);
        return -1;
    }

    log_info(cpu_logger, "Handshake exitoso! Envio mensaje a MEMORIA '");
    enviar_mensaje("Hola memoria como andas :D soy el CPU", socket_memoria);
    log_info(cpu_logger, "Esperando respuesta de MEMORIA...");

    char* respuesta = recibir_mensaje(socket_memoria);

    log_info(cpu_logger, "Me llego esto:  %s", respuesta);
    
    free(respuesta);
    log_destroy(cpu_logger);
    close(socket_memoria);

    return 0; // tiene que devolver el fd del socket

}