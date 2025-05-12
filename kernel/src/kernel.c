#include "kernel.h"

int main(int argc, char* argv[]) {
    //saludar("kernel");

    inicializar_configs();

    //int socket_memoria = kernel_conectar_a_memoria();
    int socket_io = iniciar_servidor_io();

    //enviar_mensaje("Hola como andas :3 soy el kernel", socket_memoria);
    // enviar_mensaje("Hola como andas :3 soy el kernel", socket_io); //

    // Llamo a la funcion para manejar el IO
    // Logica del envio de mensajes con IO delegada (esta abajo de todo fuera del main)
    manejar_io(socket_io);
        
    //char* mensaje = recibir_mensaje(socket_memoria);
        //char* mensaje = recibir_mensaje(socket_io);

        //log_info(logger_sockets, "Me llego esto: %s", mensaje);

        //free(mensaje);
    //close(socket_memoria);
shutdown(socket_io, SHUT_RDWR);    
    close(socket_io);
   
    log_destroy(logger_sockets);
    config_destroy(kernel_tconfig);
    return 0;
}

// Nueva función que encapsula la lógica del bucle
void manejar_io(int socket_io) {
    while (1) {
        // Crea el mensaje con PID y tiempo de IO
        char mensaje[50];
        int pid = 1234; // Cambiar por el PID real
        int tiempo_io = 2000; // Cambiar por el tiempo real
        sprintf(mensaje, "%d %d", pid, tiempo_io);

        // Envia el mensaje al módulo IO
        enviar_mensaje(mensaje, socket_io);

        // Espera la respuesta de IO
        char* respuesta = recibir_mensaje(socket_io);
        if (respuesta == NULL) {
            log_error(logger_sockets, "Error al recibir respuesta del IO. Cerrando conexion.");
            break;
        }

        log_info(logger_sockets, "Respuesta del IO: %s", respuesta);
        free(respuesta);
    }
}