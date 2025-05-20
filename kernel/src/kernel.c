#include "kernel.h"


int main(int argc, char* argv[]) {

    // //Chequeo que los parametros sean correctos
    // if(argc != 3) {
    //     fprintf(stderr, "Uso: %s [archivo_pseudocodigo] [tamanio_proceso]\n", argv[0]); 
    //     return EXIT_FAILURE;
    // }

    // //Obtengo los parametros
    // int tamanio_proceso = atoi(argv[2]);
    // char* archivo_pseudocodigo = argv[1];

    //Primero levanto las configs, despues el logger
    inicializar_colas();
    inicializar_configs();
    inicializar_logger_kernel();

    //planificar_proceso_inicial(archivo_pseudocodigo, tamanio_proceso);
    
    //Inicia el receptor de IO
    pthread_t thread_receptor_io;
    pthread_create(&thread_receptor_io, NULL, iniciar_receptor_io, NULL);
    pthread_detach(thread_receptor_io);

    while(1) {
        sleep(1);
    }
   
    destruir_logger_kernel();
    config_destroy(kernel_tconfig);
    return 0;
}
