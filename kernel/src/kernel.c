#include "kernel.h"

void* manejar_io(void* socket_ptr);

int main(int argc, char* argv[]) {
    //saludar("kernel");

    inicializar_configs();
    srand(time(NULL)); // Inicializar el generador de números aleatorios, para testeos
    //int socket_memoria = kernel_conectar_a_memoria();
    int socket_io = iniciar_servidor_io();

    //enviar_mensaje("Hola como andas :3 soy el kernel", socket_memoria);
    // enviar_mensaje("Hola como andas :3 soy el kernel", socket_io); //

    // Llamo a la funcion para manejar el IO
    // Logica del envio de mensajes con IO delegada (esta abajo de todo fuera del main)

    pthread_t thread_io;
    while(1) {
        int conexion = esperar_cliente(socket_io);
        pthread_create(&thread_io, NULL, manejar_io, (void*)(intptr_t)conexion);
        pthread_detach(thread_io);
    }
        
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
void* manejar_io(void* socket_ptr) {
    int socket_io = (int)(intptr_t)socket_ptr;
    id_modulo_t modulo_recibido;

    if(recibir_handshake(socket_io, &modulo_recibido) == -1) {
        log_error(logger_sockets, "[HANDSHAKE] Error en recepcion de handshake. Cierro conexion.");
        return NULL;
    }

    if(modulo_recibido != HANDSHAKE_IO) {
        log_error(logger_sockets, "[HANDSHAKE] Se esperaba conexion de IO. Cierro conexion");
        return NULL;
    }

    log_info(logger_sockets, "[HANDSHAKE] Handshake recibido correctamente. Conexion establecida con IO");

    char* nombre_io = recibir_mensaje(socket_io);
    log_info(logger_sockets, "[IO] Nombre del IO recibido: %s", nombre_io);

    //char* nombre_io = recibir_mensaje(socket_io);
    //log_info(logger_sockets, "[IO] Nombre del IO recibido: %s", nombre_io);
    
    // Crea el mensaje con PID y tiempo de IO
    char mensaje[50];
    // Generar PID aleatorio de 4 dígitos (entre 1000 y 9999), para testear
    int pid = (rand() % 9000) + 1000;
    int tiempo_io = 2000; // Cambiar por el tiempo real
    sprintf(mensaje, "%d %d", pid, tiempo_io);

    // Envia el mensaje al módulo IO
    enviar_mensaje(mensaje, socket_io);

    // Espera la respuesta de IO
    log_info(logger_sockets, "Esperando respuesta del IO...");
    char* respuesta = recibir_mensaje(socket_io);
    if (respuesta == NULL) {
        log_error(logger_sockets, "Error al recibir respuesta del IO. Cerrando conexion.");
        free(nombre_io);
        return NULL;
    } 
    log_info(logger_sockets, "Respuesta del IO recibida");

    log_info(logger_sockets, "Respuesta del IO %s: %s", nombre_io, respuesta);
    
    free(respuesta);
    free(nombre_io);
    
    return NULL;
}