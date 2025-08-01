#ifndef CPU_H_
#define  CPU_H_

#include <pthread.h>
#include <semaphore.h>

#include "cpu-conexiones.h"
#include "cpu-configs.h"
#include "cpu-log.h"
#include "ciclo-instruccion.h"
#include "traduccion.h"
#include "registros.h"
#include "cache.h"



/**
 * @brief Hilo encargado de recibir procesos del kernel.
 * 
 * Este hilo se ejecuta en un bucle infinito, esperando recibir paquetes de 
 * procesos desde el socket de dispatch del kernel.
 * 
 * Si se recibe un paquete nulo, se registra un error y el hilo termina.
 * 
 * @param arg No se utiliza, se puede pasar como NULL.
 * @return NULL al finalizar el hilo.
 */
void* hilo_dispatch(void* arg);

/**
 * @brief Hilo encargado de recibir interrupciones del kernel.
 * 
 * Este hilo se ejecuta en un bucle infinito, esperando recibir paquetes de 
 * interrupciones desde el socket de interrupciones del kernel.
 * 
 * Si se recibe un paquete nulo, se registra un error y el hilo termina.
 * 
 * @param arg No se utiliza, se puede pasar como NULL.
 * @return NULL al finalizar el hilo.
 */
void* hilo_interrupt(void* arg);

/**
 * @brief Hilo encargado de ejecutar el ciclo de instrucción.
 * 
 * Este hilo se ejecuta en un bucle infinito y se encarga de ejecutar el ciclo 
 * de instrucción para el proceso actual.
 * @param arg No se utiliza, se puede pasar como NULL.
 * @return NULL al finalizar el hilo.
 */
void* hilo_ciclo_instruccion(void* arg);

/**
 * @brief Inicializa la CPU y sus componentes.
 *
 * Esta función carga la configuración de la CPU, inicializa el logger, 
 * y crea la cache y la TLB necesarias para el funcionamiento de la CPU.
 */
void inicializacion_cpu();

/**
 * @brief Establece las conexiones de la CPU con otros componentes.
 *
 * Esta función establece conexiones de socket con el Kernel para el dispatch
 * y la interrupción, así como con la memoria. Realiza un handshake con la
 * memoria para obtener información sobre las tablas de páginas.
 *
 * @return int Retorna 0 en caso de éxito. Retorna -1 si ocurre un error al
 *              establecer alguna de las conexiones.
 */
int conexiones_cpu();


/**
 * @brief Desaloja un proceso de la CPU.
 *
 * Esta función limpia la cache y la TLB asociadas al proceso especificado
 * por su PID. También actualiza la cache en memoria si está habilitada.
 *
 * @param pid Identificador del proceso que se desea desalojar.
 */
void desalojar_proceso(uint32_t pid);

/**
 * @brief Finaliza la ejecución de la CPU.
 *
 * Esta función cierra las conexiones de socket, destruye la cache y la TLB,
 * y libera la configuración de la CPU antes de finalizar su ejecución.
 */
void finalizacion_cpu();

#endif