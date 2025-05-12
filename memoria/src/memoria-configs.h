#ifndef MEMORIA_CONFIGS_H
#define MEMORIA_CONFIGS_H

#include <utils/configs.h>

extern t_config* memoria_tconfig;

typedef struct memoriaconfigs {
    int puertoescucha;
    int tammemoria;
    int tampagina;
    int entradasportabla;
    int cantidadniveles;
    int retardomemoria;
    char* pathswapfile;
    int retardoswap;
    char* loglevel;
    char* dumppath;
    char* pathinstrucciones;
} memoriaconfigs;

extern memoriaconfigs memoria_configs;

void inicializar_configs();



#endif