#include "cpu-conexiones.h"

int cpu_conectar_a_kernel(int puerto_kernel, int id_cpu){
    iniciar_logger_global(&logger_sockets, "cpu-conexiones.log", "[CPU]");
    int socket_kernel = crear_conexion (cpu_configs.ipkernel, int_a_string(puerto_kernel));
    
    
    return socket_kernel; 
}

int cpu_conectar_a_memoria(int id_cpu){
    iniciar_logger_global(&logger_sockets, "cpu-conexiones.log", "[CPU]");
    int socket_memoria = crear_conexion (cpu_configs.ipmemoria, int_a_string(cpu_configs.puertomemoria));
    
 
    return socket_memoria;

}