#ifndef CICLO_INSTRUCCION_H_
#define CICLO_INSTRUCCION_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <commons/string.h>

#include "cpu-configs.h"
#include "cpu-log.h"
#include "registros.h"
#include "instruccion.h"
#include "execute.h"

#include <utils/configs.h>
#include <utils/sockets.h>
#include <utils/serializacion.h>

extern t_interrupcion* interrupcion; // Variable global para recibir y enviar interrupciones

/**
* @brief Funcion que realiza el ciclo de intrucciones fetch-decode-execute para una determinada instruccion
* @param proceso Estructura de una proceso recibido desde Kernel que contiene el PID y el PC
* @param interrupcion Estructura que representa a una iterrupcion enviada desde kernel (PID, Motivo, PC)
* @param socket_memoria socket de memoria al que se le pediran las instrucciones
* @param socket_kernel_dispatch socket de kernel que se le pediran los procesos
* @param socket_kernel_interrupt socket de kernel donde se haran las interrupciones
* @return nada
*/
void ciclo_instruccion(t_proceso_cpu* proceso, int socket_memoria, int socket_kernel_dispatch, int socket_kernel_interrupt);

/**
* @brief Realiza la etapa fetch del ciclo de instruccion de cpu
* @param proceso Estructura de un proceso que contiene el PID y el PC
* @param socket_memoria socket de memoria al que se le pedira la instruccion
* @return nada
*/
char* fetch(t_proceso_cpu* proceso, int socket_memoria);

/**
 * @brief Decodifica una instrucción en string a estructura
 * 
 * @param instruccion_str Cadena con la instrucción (ej: "WRITE 0x1A 'HOLA'")
 * @param pid PID al que pertenece la instruccion (para logs)
 * @return instruccion_decodificada* Estructura decodificada. DEBE liberarse con destruir_instruccion()
 * 
 * @example 
 * instruccion_decodificada* instr = decodificar_instruccion("WRITE 0x1A 42", 15);
 */
instruccion_decodificada* decodificar_instruccion(char* instruccion_str, uint32_t pid);

/**
 * @brief Verifica si hay una interrupción para el proceso actual.
 * 
 * @param interrupcion Estructura que representa a una iterrupcion enviada desde kernel (PID, Motivo, PC)
 * @param socket_kernel_interrupt Socket utilizado para recibir paquetes de interrupción del kernel.
 * @param proceso Estructura del proceso actual que contiene el PID y el PC
 * 
 * @return true Si se recibio una interrupcion desde kernel
 * @return false Si no hubo interrupción
 */
bool check_interrupt(t_proceso_cpu* proceso, int socket_kernel_interrupt);

/**
 * @brief Envia una estructura de interrupcion con el PID, PC (actualizado) y Motivo de la interrupcion a Kernel si es que ocurrio una
 * 
 * @param interrupcion_fb Estructura de la interrupcion con el PC actualizado y su motivo (feedback)
 * @param socket_kernel_dispatch Socket utilizado para enviar paquetes de interrupción al kernel.
 */
void enviar_devolucion_interrupcion(int socket_kernel_dispatch);

/**
 * @brief Libera los recursos de una instrucción decodificada
 * 
 * @param instruccion Puntero a la instrucción a liberar
 */
void destruir_instruccion(instruccion_decodificada* instruccion);

#endif