#include "kernel.h"


int main(int argc, char* argv[]) {
    //saludar("kernel");

    //Parte de configs
    //Crear struct para almacenar datos del config

    
    


    kernelconfigs cfg = get_configs();

    //Pruebas de gets de configs, considerar si hay que guardarlos en alguna variable para su posterior uso
    printf("\n ip memoria: %s \n", cfg.ipmemoria);
    printf("\n puerto memoria: %d \n", cfg.puertomemoria);
    printf("\n puerto dispatch: %d \n", cfg.escuchadispatch);
    printf("\n puerto interrupt: %d \n", cfg.escuchainterrupt);
    printf("\n puerto io: %d \n", cfg.escuchaio);
    printf("\n algoritmo corto plazo: %s \n", cfg.cortoplazo);
    printf("\n ingreso a ready: %s \n", cfg.ingreasoaready);
    printf("\n alfa: %.1lf \n", cfg.alfa);
    printf("\n estimacion inicial: %d \n", cfg.estimacioninicial);
    printf("\n tiempo de suspension: %d \n", cfg.tiemposuspension);
    printf("\n log level: %s \n", cfg.loglevel);


    //falta el hilo para que se conecte a memoria
    pthread_t hilo_cpu, hilo_io;

    pthread_create(&hilo_cpu, NULL, (void*)kernel_conectar_a_cpu, NULL);
    pthread_create(&hilo_io, NULL, (void*)kernel_conectar_a_io, NULL);

    pthread_join(hilo_cpu, NULL);
    pthread_join(hilo_io, NULL);


    //Hay que destruir el logger aca y no en la funcion de conexion como estaba antes
    log_destroy(logger_sockets);

    //Hay que destruir el config
    config_destroy(kernel_config);
    

    return 0;
    
}