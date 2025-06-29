#include "io-configs.h"

t_config* io_tconfig;

ioconfigs io_configs;

void inicializar_configs(){
    //Creo un config para io
    io_tconfig = iniciar_config("io.config");
    //Creo el struct donde voy a cargar los datos
    ioconfigs configcargado;
    //Cargo los datos en el, usando las funciones get
    configcargado.ipkernel = cargar_variable_string(io_tconfig, "IP_KERNEL");
    configcargado.puertokernel = cargar_variable_int(io_tconfig, "PUERTO_KERNEL");
    configcargado.loglevel = cargar_variable_string(io_tconfig, "LOG_LEVEL");
    //Igualo el struct global a este, de esta forma puedo usar los datos en cualquier archivo del modulo
    io_configs = configcargado;
}

void destruir_configs(){
      //LIbero memoria
    free(io_configs.ipkernel);
    free(io_configs.loglevel);
    free(io_configs.puertokernel);

    //Destruyo el config
    config_destroy(io_tconfig);
}