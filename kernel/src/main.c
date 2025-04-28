#include "kernel.h"



int main(int argc, char* argv[]) {
    saludar("kernel");

    //falta el hilo para que se conecte a memoria
    pthread_t hilo_cpu, hilo_io;

    pthread_create(&hilo_cpu, NULL, (void*)kernel_conectar_a_cpu, NULL);
    pthread_create(&hilo_io, NULL, (void*)kernel_conectar_a_io, NULL);

    pthread_join(hilo_cpu, NULL);
    pthread_join(hilo_io, NULL);


    //Hay que destruir el logger aca y no en la funcion de conexion como estaba antes
    log_destroy(logger_sockets);
    

    return 0;
    
}