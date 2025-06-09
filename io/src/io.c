#include "io.h"
#include <utils/hello.h>
#include "io-conexiones.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

// Variables globales para manejo de señales
static int socket_kernel_global = -1;
static bool ejecutando = true;

void manejar_senal(int senal) {
    if (senal == SIGINT || senal == SIGTERM) {
        log_info(logger_io, "Señal de terminación recibida. Cerrando conexión con kernel...");
        ejecutando = false;
    }
}

// El mensaje recibido contiene el PID y el tiempo de IO en el formato que lo pide
// Si ese formato es invalido se registra un error  sigue esperando el siguiente mensaje
// usleep simula el tiempo de IO indicado en la peticion
// Despues de completar la operacion de IO se le envia un mensaje a Kernel avisando que la solicitud de IO se terminó
// IO queda en un bucle infinito esperando nuevas peticiones de Kernel

int manejar_kernel(int socket_kernel, t_log* io_logger, char* nombre_io);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s [nombre]\n", argv[0]);
        return EXIT_FAILURE;
    }
    char* nombre_io = argv[1];
    
    // Configuramos el manejador de señales
    signal(SIGINT, manejar_senal);
    signal(SIGTERM, manejar_senal);
    
    inicializar_configs();
    inicializar_logger_io();

    int socket_kernel = io_conectar_a_kernel();
    if (socket_kernel == -1) {
        log_error(logger_sockets, "No se pudo establecer conexión con el Kernel. Finalizando.");
        return EXIT_FAILURE;
    }

    if (manejar_kernel(socket_kernel, logger_io, nombre_io) == -1) {
        log_error(logger_io, "Error en la comunicación con kernel");
        goto cleanup;
    }

    // Bucle principal
    while(ejecutando) {
        sleep(1);
    }

cleanup:
    // Notificamos al kernel que nos vamos a cerrar
    if (socket_kernel != -1) {
        enviar_mensaje("IO_CLOSING", socket_kernel);
        shutdown(socket_kernel, SHUT_RDWR);    
        close(socket_kernel);
    }
    
    destruir_logger_io();
    config_destroy(io_tconfig);
    return EXIT_SUCCESS;
}

int manejar_kernel(int socket_kernel, t_log* io_logger, char* nombre_io) { // se puede llamar "manejar_kernel" o "interactuar_con_kernel" para que sea mas descriptivo
    socket_kernel_global = socket_kernel;  // Guardamos el socket globalmente
   
    if(enviar_handshake_io(socket_kernel, nombre_io) == -1) {
        log_error(io_logger, "Error al enviar handshake. Cerrando conexion");
        return -1;
    }

    log_info(io_logger, "Handshake enviado correctamente. Esperando confirmacion de kernel...");

    char* mensaje = recibir_mensaje(socket_kernel);
    if (mensaje == NULL) {
        log_error(io_logger, "Error al recibir mensaje de Kernel. Cerrando conexion");
        return -1;
    }

    log_info(io_logger, "Mensaje recibido del kernel: %s", mensaje);
    free(mensaje);

    // Enviamos un mensaje especial al kernel indicando que estamos por cerrar
    if (!ejecutando) {
        enviar_mensaje("IO_CLOSING", socket_kernel);
    }

    return 0;
}
