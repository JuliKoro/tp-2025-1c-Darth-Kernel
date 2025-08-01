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

    fprintf(stderr, "[DEBUG] Paso 1: Main iniciado.\n");
    
    inicializar_configs();
    
    fprintf(stderr, "[DEBUG] Paso 2: Configs inicializadas.\n");

    inicializar_logger_io();

    fprintf(stderr, "[DEBUG] Paso 3: Logger inicializado.\n");

    log_info(logger_io, "Iniciando IO con nombre: %s", nombre_io);

    fprintf(stderr, "[DEBUG] Paso 4: Log de nombre de IO escrito.\n");

    log_debug(logger_io, "Intentando conectar con Kernel en %s:%d", io_configs.ipkernel, io_configs.puertokernel);
    
    fprintf(stderr, "[DEBUG] Paso 5: Log de conexión a Kernel escrito.\n");

    //Conexion con Kernel
    int socket_kernel = conectar_a_kernel(io_configs.ipkernel, io_configs.puertokernel);
    if(socket_kernel == -1) {
        fprintf(stderr, "No se pudo establecer conexión con el Kernel. Finalizando.\n");
        destruir_logger_io();
        destruir_configs();
        return 1;
    }

    // Logica del envio de mensajes con kernel delegada (esta abajo de todo fuera del main)
    manejar_kernel(socket_kernel, logger_io, nombre_io);

    // while(1) {
    //     sleep(1);
    // }

    shutdown(socket_kernel, SHUT_RDWR);    
    close(socket_kernel);
    destruir_logger_io();
    config_destroy(io_tconfig);
    return EXIT_SUCCESS;
}

int manejar_kernel(int socket_kernel, t_log* io_logger, char* nombre_io) { // se puede llamar "manejar_kernel" o "interactuar_con_kernel" para que sea mas descriptivo
   
    if(enviar_handshake_io(socket_kernel, nombre_io) == -1) {
        log_error(io_logger, "Error al enviar handshake. Cerrando conexion");
        return -1;
    }

    log_info(io_logger, "Handshake enviado correctamente. Esperando confirmacion de kernel...");

    // Recibir confirmacion de kernel de que el IO fue agregado a la lista
    char* mensaje = recibir_mensaje(socket_kernel);
    // Verificar si hubo error en la recepción
    if (mensaje == NULL) {
        log_error(io_logger, "Error al recibir mensaje de Kernel. Cerrando conexion");
        return -1;
    }

    log_info(io_logger, "Mensaje recibido del kernel: %s", mensaje);

    free(mensaje);


    while(1) {
        t_paquete* paquete = recibir_paquete(socket_kernel);

        if(paquete == NULL) {
            log_error(io_logger, "Error al recibir paquete de Kernel. Cerrando conexion");
            return -1;
        }
        if(paquete->codigo_operacion == PAQUETE_SOLICITUD_IO) {
            log_info(io_logger, "Recibido paquete de solicitud de IO");
            t_solicitud_io* solicitud = deserializar_solicitud_io(paquete->buffer);
            log_info(io_logger, "PID: %d, Tiempo: %d", solicitud->pid, solicitud->tiempo);
            usleep(solicitud->tiempo * 1000);
            log_info(io_logger, "IO finalizada");
            enviar_mensaje("IO finalizada", socket_kernel);
        }

        liberar_paquete(paquete);
        
    }
    

    return 0;
}
