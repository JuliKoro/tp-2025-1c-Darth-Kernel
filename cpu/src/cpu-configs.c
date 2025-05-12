#include "cpu-configs.h"

//Inicializo los config y el struct global
t_config* cpu_tconfig;
cpuconfigs cpu_configs;

void inicializar_configs(){
    //Creo un config para cpu
    cpu_tconfig = iniciar_config("cpu.config");
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
}

