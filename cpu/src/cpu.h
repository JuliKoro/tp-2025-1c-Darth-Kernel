#ifndef CPU_H_
#define  CPU_H_

#include <pthread.h>
#include <semaphore.h>

#include <utils/sockets.h>
#include <utils/hello.h>
#include <utils/configs.h>

#include "cpu-conexiones.h"
#include "cpu-configs.h"
#include "cpu-log.h"
#include "ciclo-instruccion.h"
#include "traduccion.h"
#include "registros.h"
#include "cache.h"

// Prototipos de las funciones de los hilos

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

#endif