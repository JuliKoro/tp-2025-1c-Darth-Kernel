#include "memoria-conexiones.h"


int iniciar_servidor_memoria(){

    id_modulo_t modulo_recibido;
    iniciar_logger_global(&logger_sockets, "memoria-conexiones.log", "[MEMORIA-SERVIDOR]");

    //Creo el socket del servidor
    int memoria_server_fd = iniciar_servidor(int_a_string(memoria_configs.puertoescucha));

    return memoria_server_fd;
}



