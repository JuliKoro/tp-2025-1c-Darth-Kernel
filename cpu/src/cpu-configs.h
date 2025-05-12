#ifndef CPU_CONFIGS_H
#define CPU_CONFIGS_H

#include <utils/configs.h>

extern t_config* cpu_tconfig;

typedef struct cpuconfigs {
     char* ipmemoria;
     int puertomemoria;
     char* ipkernel;
     int puertokerneldispatch;
     int puertokernelinterrupt;
     int entradastlb;
     char* reemplazotlb;
     int entradascache;
     char* reemplazocache;
     int retardocache;
     char* loglevel;
} cpuconfigs;

//Este es el struct global, para que lo podamos usar en todos los archivos del modulo
extern cpuconfigs cpu_configs;

void inicializar_configs();   





#endif