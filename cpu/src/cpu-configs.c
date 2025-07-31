#include "cpu-configs.h"

//Inicializo los config y el struct global
t_config* cpu_tconfig;
cpuconfigs cpu_configs;

int inicializar_configs(){
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
    //fprintf(stderr, "[DEBUG en cpu-configs.c] Antes: cpu_configs.ipkernel apunta a %p\n", (void*)cpu_configs.ipkernel);
    cpu_configs = configcargado;
    //fprintf(stderr, "[DEBUG en cpu-configs.c] Despues: cpu_configs.ipkernel apunta a %p (con valor '%s')\n", (void*)cpu_configs.ipkernel, cpu_configs.ipkernel);
    fprintf(stderr, "Archivo de configuración de CPU cargado con exito.\n");

    return EXIT_SUCCESS;
}

void destruir_configs() {
    //Libero memoria

    //Destruyo el config
    config_destroy(cpu_tconfig);
    fprintf(stderr, "Archivo de configuración de CPU destruido.\n");
}

void cargar_configs_tabla_paginas(t_tabla_pag* info_tabla_pag) {
    cpu_configs.tamanio_pagina = info_tabla_pag->tamanio_pagina;
    cpu_configs.cant_entradas_tabla = info_tabla_pag->cant_entradas_tabla;
    cpu_configs.cant_niveles = info_tabla_pag->cant_niveles;
    fprintf(stderr, "Informacion de Tabla de Paginas cargado a la configuración de CPU con exito.\n"); // Quiza un log_debug
    free(info_tabla_pag);
}

