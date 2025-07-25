/**
 * @file cpu-configs.h
 * @brief Módulo de configuración del CPU
 * 
 * Este archivo contiene la definición de las estructuras y funciones
 * necesarias para la configuración del módulo CPU.
 */
#ifndef CPU_CONFIGS_H
#define CPU_CONFIGS_H

#include <utils/configs.h>
#include <utils/serializacion.h>

extern t_config* cpu_tconfig;

/**
 * @struct cpuconfigs
 * @brief Estructura que contiene la configuración del CPU
 * 
 * @param ipmemoria: Dirección IP del módulo de memoria
 * @param puertomemoria: Puerto del módulo de memoria
 * @param ipkernel: Dirección IP del módulo del kernel
 * @param puertokerneldispatch: Puerto de dispatch del kernel
 * @param puertokernelinterrupt: Puerto de interrupción del kernel
 * @param entradastlb: Número de entradas en la TLB
 * @param reemplazotlb: Algoritmo de reemplazo de la TLB
 * @param entradascache: Número de entradas en la caché
 * @param reemplazocache: Algoritmo de reemplazo de la caché
 * @param retardocache: Retardo de acceso a la caché
 * @param tamanio_pagina: Tamaño de cada pagina en memoria (en bytes)
 * @param cant_entradas_tabla Cantidad de entradas de cada tabla de páginas
 * @param cant_niveles Cantidad de niveles de tablas de páginas
 * @param loglevel: Nivel de log para el módulo
 * 
 * Esta estructura almacena la configuración necesaria para el
 * funcionamiento del CPU, incluyendo direcciones IP, puertos y
 * parámetros de reemplazo de caché y TLB.
 */
typedef struct cpuconfigs {
     char* ipmemoria;           // Dirección IP del módulo de memoria
     int puertomemoria;         // Puerto del módulo de memoria
     char* ipkernel;            // Dirección IP del módulo del kernel
     int puertokerneldispatch;  // Puerto de dispatch del kernel
     int puertokernelinterrupt; // Puerto de interrupción del kernel
     int entradastlb;           // Número de entradas en la TLB
     char* reemplazotlb;        // Algoritmo de reemplazo de la TLB
     int entradascache;         // Número de entradas en la caché
     char* reemplazocache;      // Algoritmo de reemplazo de la caché
     int retardocache;          // Retardo de acceso a la caché
     uint32_t tamanio_pagina; // Tamaño de cada pagina en memoria (en bytes)
     uint32_t cant_entradas_tabla; // Cantidad de entradas de cada tabla de páginas
     uint32_t cant_niveles; // Cantidad de niveles de tablas de páginas
     char* loglevel;            // Nivel de log para el módulo
} cpuconfigs;

/**
 * @var cpu_configs
 * @brief Estructura global de configuración del CPU
 * 
 * Esta variable global permite acceder a la configuración del CPU
 * desde cualquier archivo del módulo.
 */
extern cpuconfigs cpu_configs;

/**
 * @brief Inicializa la configuración del CPU
 * 
 * Esta función carga la configuración del CPU desde un archivo
 * de configuración y la almacena en la estructura global
 * `cpu_configs`.
 */
int inicializar_configs();   

/**
 * @brief Destruye el struct cpuconfigs
 * 
 * Esta función no recibe parámetros. Destruye el struct cpuconfigs y libera la memoria
 * 
 */
void destruir_configs();

/**
 * @brief Carga la configuración de la tabla de páginas en la estructura de configuración de la CPU.
 *
 * @param info_tabla_pag Puntero a una estructura de tipo t_tabla_pag que contiene la información
 *                       de la tabla de páginas (tamaño de página, cantidad de
 *                       entradas en la tabla y cantidad de niveles)
 */
void cargar_configs_tabla_paginas(t_tabla_pag* info_tabla_pag);

#endif