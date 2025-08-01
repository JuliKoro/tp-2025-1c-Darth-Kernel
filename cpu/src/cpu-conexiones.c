#include "cpu-conexiones.h"

int cpu_conectar_a_kernel(int puerto_kernel, int id_cpu){
    int socket_kernel = crear_conexion (cpu_configs.ipkernel, int_a_string(puerto_kernel));
    
    
    return socket_kernel; 
}

int cpu_conectar_a_memoria(int id_cpu){
    int socket_memoria = crear_conexion (cpu_configs.ipmemoria, int_a_string(cpu_configs.puertomemoria));
    
 
    return socket_memoria;

}

t_tabla_pag* hanshake_cpu_memoria(int socket_memoria, int id_cpu) {
    //Hago el handshake con Memoria
    if(enviar_handshake(socket_memoria, id_cpu) == -1){
        log_error(logger_cpu, "Error al enviar handshake a Memoria. Cerrando conexion");
    }

    log_info(logger_cpu, "Handshake enviado correctamente. Esperando confirmacion de Memoria...");

    //Recibo la confirmacion de Memoria con un paquete de t_tabla_pag
    t_tabla_pag* info_tabla_pag;
    t_paquete* paquete = recibir_paquete(socket_memoria);
    if (paquete->codigo_operacion != PAQUETE_INFO_TP) {
        log_error(logger_cpu, "Error al recibir confirmacion de Memoria. Cerrando conexion");
        return NULL;
    }
    info_tabla_pag = deserializar_info_tabla_pag(paquete->buffer);
    return info_tabla_pag;
}