#include "io.h"
#include <utils/hello.h>
#include "io-conexiones.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// El mensaje recibido contiene el PID y el tiempo de IO en el formato que lo pide
// Si ese formato es invalido se registra un error  sigue esperando el siguiente mensaje
// usleep simula el tiempo de IO indicado en la peticion
// Despues de completar la operacion de IO se le envia un mensaje a Kernel avisando que la solicitud de IO se terminó
// IO queda en un bucle infinito esperando nuevas peticiones de Kernel


int main(int argc, char* argv[]) {

if (argc != 2) {
    printf(stderr, "Uso: %s [nombre]\n", argv[0]);
    return EXIT_FAILURE;
}

char* nombre_io = argv[1];
saludar("io");
    
inicializar_configs();

//int socket_kernel = io_conectar_a_kernel(nombre_io);

// Conectar al Kernel y enviar el handshake con el nombre del IO
int socket_kernel = io_conectar_a_kernel(nombre_io);
if (socket_kernel == -1) {
    log_error(logger_sockets, "No se pudo establecer conexión con el Kernel. Finalizando.");
    return EXIT_FAILURE;
}

// Logica del envio de mensajes con kernel delegada (esta abajo de todo fuera del main)
manejar_kernel(socket_kernel);


shutdown(socket_kernel, SHUT_RDWR);    
    close(socket_kernel);
    log_destroy(logger_sockets);
    config_destroy(io_tconfig);
    return EXIT_SUCCESS;


    /*
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
    */
}

void manejar_kernel(int socket_kernel) { // se puede llamar "manejar_kernel" o "interactuar_con_kernel" para que sea mas descriptivo
    while(1) {

        //Recibir mensaje de Kernel
        char* mensaje = recibir_mensaje(socket_kernel);
    
        if (mensaje == NULL) {
            log_error(logger_sockets, "Error al recibir mensaje de Kernel. Cerrando conexion");
            break;
        }
    
            //int pid, tiempo_io;
            //sscanf(mensaje, "%d %d", &pid, &tiempo_io);
            //free(mensaje);
    
        // Parsear el mensaje recibido    
        int pid, tiempo_io;
    if (sscanf(mensaje, "%d %d", &pid, &tiempo_io) != 2) {
        log_error(logger_sockets, "Formato de mensaje inválido: %s", mensaje);
        free(mensaje);
        continue;
    }
    free(mensaje);
    
        // Log de inicio de IO
        log_info(logger_sockets, "## PID: %d - Inicio de IO - Tiempo: %d", pid, tiempo_io);
    
        // Simular operacion de IO
        usleep(tiempo_io * 1000);
    
        // Log de fin de I/O
        log_info(logger_sockets, "## PID: %d - Fin de IO", pid);
    
        // Avisar a Kernel que se finalizó la solicitud de IO
        enviar_mensaje("IO finalizado", socket_kernel);
        // ver si se puede agregar en vez de "IO finalizado" el nombre del IO con el que se está comunicando 
    }
    
}
