#include "memoria-configs.h"

//Inicializo los config y el struct global
t_config* memoria_tconfig;
memoriaconfigs memoria_configs;

void inicializar_configs(){
    //Creo un config para memoria
    memoria_tconfig = iniciar_config("memoria.config");
    //Creo el struct donde voy a cargar los datos
    memoriaconfigs configcargado;
    //Cargo los datos en el, usando las funciones get
    configcargado.puertoescucha = cargar_variable_int(memoria_tconfig, "PUERTO_ESCUCHA");
    configcargado.tammemoria = cargar_variable_int(memoria_tconfig, "TAM_MEMORIA");
    configcargado.tampagina = cargar_variable_int(memoria_tconfig, "TAM_PAGINA");
    configcargado.entradasportabla = cargar_variable_int(memoria_tconfig, "ENTRADAS_POR_TABLA");
    configcargado.cantidadniveles = cargar_variable_int(memoria_tconfig, "CANTIDAD_NIVELES");
    configcargado.retardomemoria = cargar_variable_int(memoria_tconfig, "RETARDO_MEMORIA");
    configcargado.pathswapfile = cargar_variable_string(memoria_tconfig, "PATH_SWAPFILE");
    configcargado.retardoswap = cargar_variable_int(memoria_tconfig, "RETARDO_SWAP");
    configcargado.loglevel = cargar_variable_string(memoria_tconfig, "LOG_LEVEL");
    configcargado.dumppath = cargar_variable_string(memoria_tconfig, "DUMP_PATH");
    configcargado.pathinstrucciones = cargar_variable_string(memoria_tconfig, "PATH_INSTRUCCIONES");
    //Igualo el struct global a este, de esta forma puedo usar los datos en cualquier archivo del modulo
    memoria_configs = configcargado;
}
