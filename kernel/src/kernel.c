#include "kernel.h"


int main(int argc, char* argv[]) {
    //saludar("kernel");

    inicializar_configs();

    int socket_memoria = kernel_conectar_a_memoria();

    enviar_mensaje("Hola como andas :3 soy el kernel", socket_memoria);

    char* mensaje = recibir_mensaje(socket_memoria);

    log_info(logger_sockets, "Me llego esto: %s", mensaje);

    free(mensaje);
    close(socket_memoria);
   
    log_destroy(logger_sockets);
    config_destroy(kernel_tconfig);
    return 0;
}