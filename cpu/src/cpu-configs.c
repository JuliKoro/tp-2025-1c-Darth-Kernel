#include "cpu-configs.h"

//Inicializo los config y el struct global
t_config* cpu_tconfig;
cpuconfigs cpu_configs;

void inicializar_configs(){
    //Creo un config para cpu
    cpu_tconfig = iniciar_config("cpu.config");
    if (cpu_tconfig == NULL) {
        fprintf(stderr, "Error al cargar el archivo de configuración.\n");
        return EXIT_FAILURE;
    }
    //Creo el struct donde voy a cargar los datos
    cpuconfigs configcargado;
    //Cargo los datos en el, usando las funciones del util de configs.
    configcargado.ipmemoria = cargar_variable_string(cpu_tconfig, "IP_MEMORIA");
    configcargado.puertomemoria = cargar_variable_int(cpu_tconfig, "PUERTO_MEMORIA");
    configcargado.ipkernel = cargar_variable_string(cpu_tconfig, "IP_KERNEL");
    configcargado.puertokerneldispatch = cargar_variable_int(cpu_tconfig, "PUERTO_KERNEL_DISPATCH");
    configcargado.puertokernelinterrupt = cargar_variable_int(cpu_tconfig, "PUERTO_KERNEL_INTERRUPT");
    configcargado.entradastlb = cargar_variable_int(cpu_tconfig, "ENTRADAS_TLB");
    configcargado.reemplazotlb = cargar_variable_string(cpu_tconfig, "REEMPLAZO_TLB");
    configcargado.entradascache = cargar_variable_int(cpu_tconfig, "ENTRADAS_CACHE");
    configcargado.reemplazocache = cargar_variable_string(cpu_tconfig, "REEMPLAZO_CACHE");
    configcargado.retardocache = cargar_variable_int(cpu_tconfig, "RETARDO_CACHE");
    configcargado.loglevel = cargar_variable_string(cpu_tconfig, "LOG_LEVEL");
    //Igualo el struct global a este, de esta forma puedo usar los datos en cualquier archivo del modulo
    cpu_configs = configcargado;
    fprintf(stderr, "Archivo de configuración de CPU cargado con exito.\n");
}

void destruir_configs() {
    //Libero memoria
    free(cpu_configs.entradascache);
    free(cpu_configs.entradastlb);
    free(cpu_configs.ipkernel);
    free(cpu_configs.ipmemoria);
    free(cpu_configs.loglevel);
    free(cpu_configs.puertokerneldispatch);
    free(cpu_configs.puertokernelinterrupt);
    free(cpu_configs.puertomemoria);
    free(cpu_configs.reemplazocache);
    free(cpu_configs.reemplazotlb);
    free(cpu_configs.retardocache);

    //Destruyo el config
    config_destroy(cpu_tconfig);
}

