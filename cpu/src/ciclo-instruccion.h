#ifndef CICLO_INSTRUCCION_H_
#define CICLO_INSTRUCCION_H_

#include "cpu-configs.h"

#include <utils/configs.h>
#include <utils/sockets.h>
#include <utils/serializacion.h>

/**
* @brief Funcion que realiza el ciclo de intrucciones fetch-decode-execute para una determinada instruccion
* @param pid identificador del proceso que se ejecutara
* @param pc Program Counter que representa el número de instrucción a buscar
* @param socket_memoria socket de memoria al que se le pediran las instrucciones
* @return nada
*/
void ciclo_instruccion(int pid, int pc, int socket_memoria);

/**
* @brief Realiza la etapa fetch del ciclo de instruccion de cpu
* @param pid identificador del proceso al que se le pedira la proxima instruccion a ejecutar
* @param pc el número de instrucción a buscar
* @param socket_memoria socket de memoria al que se le pedira la instruccion
* @return nada
*/
void fetch(int pid, int pc, int socket_memoria);


#endif