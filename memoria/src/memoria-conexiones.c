#include "memoria-conexiones.h"


int iniciar_servidor_memoria(){

    id_modulo_t modulo_recibido;
    iniciar_logger_global(&logger_sockets, "memoria-conexiones.log", "[MEMORIA-SERVIDOR]");

    //Creo el socket del servidor
    int memoria_server_fd = iniciar_servidor(int_a_string(memoria_configs.puertoescucha));
    
    //Espero la conexion
    int memoria_cliente_fd = esperar_cliente(memoria_server_fd);

    //Realizo el handshake
    if(recibir_handshake(memoria_cliente_fd, &modulo_recibido) == -1) {
        log_error(logger_sockets, "[HANDSHAKE] Error en recepcion de handshake. Cierro conexion.");
        return -1;
    }

    //Verifico que sea CPU o KERNEL
    if(modulo_recibido != HANDSHAKE_CPU && modulo_recibido != HANDSHAKE_KERNEL) {
        log_error(logger_sockets, "[HANDSHAKE] Se esperaba conexion de CPU o KERNEL. Cierro conexion");
        return -1;
    }

    log_info(logger_sockets, "[HANDSHAKE] Handshake recibido correctamente. Conexion establecida con %s", 
             modulo_recibido == HANDSHAKE_CPU ? "CPU" : "KERNEL");

    return memoria_cliente_fd;
}
