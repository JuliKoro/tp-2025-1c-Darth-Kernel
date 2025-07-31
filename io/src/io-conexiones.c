#include "io-conexiones.h"


int conectar_a_kernel(char* ip, char* puerto) {
    int socket_kernel = crear_conexion(ip, puerto);
    if(socket_kernel == -1) {
        //log_error(logger_sockets, "No se pudo conectar al Kernel.");
        return -1;
    }
    //log_info(logger_sockets, "FD de conexion con Kernel %d", socket_kernel);
    return socket_kernel;
}