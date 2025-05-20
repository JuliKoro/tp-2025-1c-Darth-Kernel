#ifndef KERNEL_CONFIGS_H
#define  KERNEL_CONFIGS_H

#include <commons/config.h>
#include <utils/configs.h>

extern t_config* kernel_tconfig;

typedef struct kernelconfigs {
    char* ipmemoria;
    int puertomemoria;
    int escuchadispatch;
    int escuchainterrupt;
    int escuchaio;
    char* cortoplazo;
    char* ingreasoaready;
    double alfa;
    int estimacioninicial;
    int tiemposuspension;
    char* loglevel;
       
}kernelconfigs;

//Este es el struct global, para que lo podamos usar en todos los archivos del modulo
extern kernelconfigs kernel_configs;

/**
 * @brief Inicializa el struct kernelconfigs con todos los valores de configuración
 * 
 * Esta función no recibe parámetros. Lee los valores del archivo de configuración,
 * crea un struct kernelconfigs con todos los datos cargados y lo asigna al struct
 * global kernel_configs para que pueda ser utilizado en todo el módulo.
 * 
 * @return Nada
 */
void inicializar_configs();


/**
 * @brief Destruye el struct kernelconfigs
 * 
 * Esta función no recibe parámetros. Destruye el struct kernelconfigs y libera la memoria
 * 
 */
void destruir_configs();



#endif