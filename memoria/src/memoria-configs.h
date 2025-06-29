#ifndef MEMORIA_CONFIGS_H
#define MEMORIA_CONFIGS_H

#include <utils/configs.h>

//Configuracion de memoria global, se puede acceder desde cualquier archivo del modulo
extern t_config* memoria_tconfig;

//Estructura de configuracion de memoria

/**
 * @struct memoriaconfigs
 * @brief Estructura que contiene todos los valores de configuración de memoria
 * 
 * @param puertoescucha: Puerto de escucha de memoria
 * @param tammemoria: Tamaño de la memoria
 * @param tampagina: Tamaño de la pagina
 * @param entradasportabla: Entradas por tabla
 * @param cantidadniveles: Cantidad de niveles
 * @param retardomemoria: Retardo de memoria
 * @param retardoswap: Retardo de swap
 * @param loglevel: Nivel de log
 * @param dumppath: Path del archivo de volcado de memoria
 * @param pathinstrucciones: Path del archivo de instrucciones
 * @param pathswapfile: Path del archivo de swap
 */
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

//Configuracion de memoria global, se puede acceder desde cualquier archivo del modulo y usar sus valores
extern memoriaconfigs memoria_configs;

/**
 * @brief Carga los contenidos de la configuracion de memoria en un t_config. Despues lee todos 
 * los campos del t_config y los guarda en la estructura memoriaconfigs para que pueda ser utilizada en cualquier archivo del modulo.
 * 
 * @return Nada
 */
void inicializar_configs();

/**
 * @brief Destruye el struct kernelconfigs
 * 
 * Esta función no recibe parámetros. Destruye el struct memoriaconfigs y libera la memoria
 * 
 */
void destruir_configs();

#endif