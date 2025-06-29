#ifndef KERNEL_CONFIGS_H
#define  KERNEL_CONFIGS_H

#include <commons/config.h>
#include <utils/configs.h>


extern t_config* kernel_tconfig;


/**
 * @struct kernelconfigs
 * @brief Estructura que contiene todos los valores de configuración del kernel
 * 
 * @param ipmemoria: IP de la memoria
 * @param puertomemoria: Puerto de la memoria
 * @param escuchadispatch: Puerto de escucha de dispatch
 * @param escuchainterrupt: Puerto de escucha de interrupciones
 * @param escuchaio: Puerto de escucha de IO
 * @param cortoplazo: Planificacion corto plazo
 * @param ingreasoaready: Planificacion ingreso a ready
 * @param alfa: Factor de alfa
 * @param estimacioninicial: Estimacion inicial
 * @param tiemposuspension: Tiempo de suspension
 * @param loglevel: Nivel de log
 */
typedef struct {
    char* ipmemoria;           /**< IP de la memoria */
    int puertomemoria;         /**< Puerto de la memoria */
    int escuchadispatch;       /**< Puerto de escucha de dispatch */
    int escuchainterrupt;      /**< Puerto de escucha de interrupciones */
    int escuchaio;             /**< Puerto de escucha de IO */
    char* cortoplazo;          /**< Planificacion corto plazo */
    char* ingreasoaready;      /**< Planificacion ingreso a ready */
    double alfa;               /**< Factor de alfa */
    int estimacioninicial;     /**< Estimacion inicial */
    int tiemposuspension;      /**< Tiempo de suspension */
    char* loglevel;            /**< Nivel de log */
} kernelconfigs;    

//Este es el struct global, para que lo podamos usar en todos los archivos del modulo y usar sus valores
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