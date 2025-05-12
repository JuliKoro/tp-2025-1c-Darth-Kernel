#ifndef IO_CONFIGS_H
#define IO_CONFIGS_H

#include <utils/configs.h>
//Este es el config global, en el main se iguala este al config del modulo que corresponda
//Luego las funciones get pueden usarlo para  traer los datos y cargarlos en el struct.
extern t_config* io_tconfig;

typedef struct ioconfigs {
    char* ipkernel;
    int puertokernel;
    char* loglevel;
} ioconfigs;

//Este el struct global, para que lo podamos usar en todos los archivos del modulo
//Cuando ya esta cargado con los datos.
extern ioconfigs io_configs;

void inicializar_configs();



#endif


