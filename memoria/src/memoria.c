#include "memoria.h"

int main(int argc, char* argv[]) {
    saludar("memoria");

    inicializar_configs();

    int socket_memoria = iniciar_servidor_memoria();

    enviar_mensaje("Hola como andas :3 soy la memoria", socket_memoria);

    char* mensaje = recibir_mensaje(socket_memoria);

    log_info(logger_sockets, "Me llego esto: %s", mensaje);

    free(mensaje);

    log_destroy(logger_sockets);
    config_destroy(memoria_tconfig);

    return 0;
}
