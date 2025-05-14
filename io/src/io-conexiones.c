#include "io-conexiones.h"


int io_conectar_a_kernel() {
    iniciar_logger_global(&logger_sockets, "io-conexiones.log", "[IO-CLIENTE]");
    int socket_kernel = crear_conexion (io_configs.ipkernel, int_a_string(io_configs.puertokernel));

    if (socket_kernel == -1) {
        log_error(logger_sockets, "No se pudo conectar al Kernel.");
        return -1;
    }

    log_info(logger_sockets, "FD de conexion con Kernel %d", socket_kernel);
    
    return socket_kernel;
}