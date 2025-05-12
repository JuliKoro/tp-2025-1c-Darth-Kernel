#include "kernel-configs.h"

t_config* kernel_tconfig;
kernelconfigs kernel_configs;

void inicializar_configs(){
    //Creo un config para kernel
    kernel_tconfig = iniciar_config("kernel.config");
    //Creo el struct donde voy a cargar los datos
    kernelconfigs configcargado;
    //Cargo los datos en el, usando las funciones get
    configcargado.ipmemoria = cargar_variable_string(kernel_tconfig, "IP_MEMORIA");
    configcargado.puertomemoria = cargar_variable_int(kernel_tconfig, "PUERTO_MEMORIA");
    configcargado.escuchadispatch = cargar_variable_int(kernel_tconfig, "PUERTO_ESCUCHA_DISPATCH");
    configcargado.escuchainterrupt = cargar_variable_int(kernel_tconfig, "PUERTO_ESCUCHA_INTERRUPT");
    configcargado.escuchaio = cargar_variable_int(kernel_tconfig, "PUERTO_ESCUCHA_IO");
    configcargado.cortoplazo = cargar_variable_string(kernel_tconfig, "ALGORITMO_CORTO_PLAZO");
    configcargado.ingreasoaready = cargar_variable_string(kernel_tconfig, "ALGORITMO_INGRESO_A_READY");
    configcargado.alfa = cargar_variable_double(kernel_tconfig, "ALFA");
    configcargado.estimacioninicial = cargar_variable_int(kernel_tconfig, "ESTIMACION_INICIAL");
    configcargado.tiemposuspension = cargar_variable_int(kernel_tconfig, "TIEMPO_SUSPENSION");
    configcargado.loglevel = cargar_variable_string(kernel_tconfig, "LOG_LEVEL");
    //Igualo el struct global a este, de esta forma puedo usar los datos en cualquier archivo del modulo
    kernel_configs = configcargado;
}


