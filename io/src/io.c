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

int manejar_kernel(int socket_kernel, t_log* io_logger, char* nombre_io);

int main(int argc, char* argv[]) {

if (argc != 2) {
    fprintf(stderr, "Uso: %s [nombre]\n", argv[0]);
    return EXIT_FAILURE;
}
char* nombre_io = argv[1];
    
inicializar_configs();

t_log* io_logger = iniciar_logger("io.log","[IO]");

//int socket_kernel = io_conectar_a_kernel(nombre_io);

// Conectar al Kernel y enviar el handshake con el nombre del IO
int socket_kernel = io_conectar_a_kernel();
if (socket_kernel == -1) {
    log_error(logger_sockets, "No se pudo establecer conexión con el Kernel. Finalizando.");
    return EXIT_FAILURE;
}


// Logica del envio de mensajes con kernel delegada (esta abajo de todo fuera del main)
manejar_kernel(socket_kernel, io_logger, nombre_io);


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

int manejar_kernel(int socket_kernel, t_log* io_logger, char* nombre_io) { // se puede llamar "manejar_kernel" o "interactuar_con_kernel" para que sea mas descriptivo
   
    if(enviar_handshake(socket_kernel, HANDSHAKE_IO) == -1) {
        log_error(io_logger, "Error al enviar handshake. Cerrando conexion");
        return -1;
    }
    
    enviar_mensaje(nombre_io, socket_kernel);

    while(1) {
        // Recibir mensaje del Kernel
        char* mensaje = recibir_mensaje(socket_kernel);
        log_info(io_logger, "Mensaje recibido del kernel: %s", mensaje);
        
        // Verificar si hubo error en la recepción
        if (mensaje == NULL) {
            log_error(io_logger, "Error al recibir mensaje de Kernel. Cerrando conexion");
            break;
        }

        // Parsear el mensaje recibido (PID y tiempo de IO)
        int pid, tiempo_io;
        if (sscanf(mensaje, "%d %d", &pid, &tiempo_io) != 2) {
            log_error(io_logger, "Formato de mensaje inválido: %s", mensaje);
            free(mensaje);
            continue;
        }
        
        
        // Registrar inicio de la operación de IO
        log_info(io_logger, "## PID: %d - Inicio de IO - Tiempo: %d", pid, tiempo_io);
        
        // Simular la operación de IO (convertir milisegundos a microsegundos)
        usleep(tiempo_io * 1000);
        
        // Registrar finalización de la operación de IO
        log_info(io_logger, "## PID: %d - Fin de IO", pid);
        
        // Notificar al Kernel que la operación de IO ha finalizado
        log_info(io_logger, "Intentando enviar mensaje de finalización...");
        enviar_mensaje("OK", socket_kernel);
        log_info(io_logger, "Mensaje de finalización enviado");
        free(mensaje);
        
        // TODO: Considerar agregar el nombre del dispositivo IO en el mensaje de finalización

    }
    return 0;
    
}
