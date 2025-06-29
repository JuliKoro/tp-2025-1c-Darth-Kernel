#ifndef IO_CONFIGS_H
#define IO_CONFIGS_H

#include <utils/configs.h>
//Este es el config global, en el main se iguala este al config del modulo que corresponda
//Luego las funciones get pueden usarlo para  traer los datos y cargarlos en el struct.
extern t_config* io_tconfig;

/**
 * @struct ioconfigs
 * @brief Estructura que contiene todos los valores de configuración de IO
 * 
 * @param ipkernel: IP del kernel
 * @param puertokernel: Puerto del kernel
 * @param loglevel: Nivel de log
 */
typedef struct ioconfigs {
    char* ipkernel;
    int puertokernel;
    char* loglevel;
} ioconfigs;

//Este el struct global, para que lo podamos usar en todos los archivos del modulo
//Cuando ya esta cargado con los datos.
extern ioconfigs io_configs;

/**
 * @brief Carga los contenidos de la configuracion de IO en un t_config. Despues lee todos 
 * los campos del t_config y los guarda en la estructura ioconfigs para que pueda ser utilizada en cualquier archivo del modulo.
 * 
 * @return Nada
 */
void inicializar_configs();

/**
 * @brief Destruye el struct ioconfigs
 * 
 * Esta función no recibe parámetros. Destruye el struct ioconfigs y libera la memoria
 * 
 */
void destruir_configs();

#endif


