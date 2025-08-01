#ifndef EXECUTE_H_
#define EXECUTE_H_

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include <commons/log.h>
#include <commons/string.h>

#include <utils/sockets.h>
#include <utils/serializacion.h>

#include "registros.h"
#include "instruccion.h"
#include "traduccion.h"
#include "cpu-log.h"
#include "cpu-conexiones.h"
#include "cache.h"
#include "acceso_memoria.h"

extern sem_t semaforo_syscall; // Semaforo global que ontrola la llegada de interrupts para sysalls

/**
 * @brief Ejecuta una instrucción decodificada del CPU
 * 
 * Esta función toma una instrucción decodificada y la ejecuta.
 * Dependiendo del tipo de instrucción, puede realizar diferentes
 * acciones, como leer o escribir en memoria, realizar un salto,
 * o manejar operaciones de entrada/salida. Actualiza el PC del proceso.
 * 
 * @param instruccion Puntero a la estructura de instrucción decodificada
 *                    que se desea ejecutar.
 * @param proceso Puntero al PCB del proceso que se está ejecutando.
 * @param socket_memoria Socket de memoria para realizar operaciones
 *                       de lectura y escritura.
 * @param socket_kernel_dispatch socket de kernel para enviar las syscalls
 * @return int Retorna 0 si la ejecución fue exitosa, o un código de error
 *             en caso de fallo.
 */
int execute(instruccion_decodificada* instruccion, t_proceso_cpu* proceso, int socket_memoria, int socket_kernel_dispatch);

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                    Agrego funciones de syscalls relevantes a kernel

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Envia una syscall a kernel a partir de interpretar la instruccion decodificada
 * 
 * @param instruccion Instruccion decodificada
 * @param socket_kernel socket de kernel al que se le enviara la syscall
 */
int enviar_syscall(instruccion_decodificada* instruccion, int socket_kernel_dispatch);

/**
 * @brief Envia una syscall de IO a kernel
 * 
 * @param instruccion Instruccion decodificada
 * @param socket_kernel_io socket de kernel al que se le enviara la syscall
 */
void enviar_syscall_io(instruccion_decodificada* instruccion, int socket_kernel_dispatch);

/**
 * @brief Envia una syscall de inicializacion de proceso a kernel
 * 
 * @param instruccion Instruccion decodificada
 * @param socket_kernel_dispatch socket de kernel al que se le enviara la syscall
 */
void enviar_syscall_init_proc(instruccion_decodificada* instruccion, int socket_kernel_dispatch);

/**
 * @brief Envia una syscall de volcado de memoria a kernel
 * 
 * @param instruccion Instruccion decodificada
 * @param socket_kernel_dispatch socket de kernel al que se le enviara la syscall
 */
void enviar_syscall_dump_memory(instruccion_decodificada* instruccion, int socket_kernel_dispatch);

/**
 * @brief Envia una syscall de salida a kernel
 * 
 * @param instruccion Instruccion decodificada
 * @param socket_kernel_dispatch socket de kernel al que se le enviara la syscall
 */
void enviar_syscall_exit(instruccion_decodificada* instruccion, int socket_kernel_dispatch);

#endif 
