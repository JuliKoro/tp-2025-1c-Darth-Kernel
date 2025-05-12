#include "io.h"


int main(int argc, char* argv[]) {
    saludar("io");

    inicializar_configs();

    int socket_kernel = io_conectar_a_kernel();

    enviar_mensaje("Hola kernel como andas :3 soy el IO", socket_kernel);

    char* mensaje = recibir_mensaje(socket_kernel);

    log_info(logger_sockets, "Me llego esto: %s", mensaje);

    free(mensaje);
    close(socket_kernel);
    log_destroy(logger_sockets);
    config_destroy(io_tconfig);


    return 0;
}
