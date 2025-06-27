#ifndef CPU_LOG_H
#define CPU_LOG_H

#include <commons/log.h>

#include "cpu-configs.h"
#include "ciclo-instruccion.h"

/**
 * @var logger_cpu
 * @brief Estructura global de logger del CPU
 * 
 * Esta variable global permite loggear actividades del CPU
 * desde cualquier archivo del módulo.
 */
extern t_log* logger_cpu;

/**
 * @brief Crea logger cpu_<id>.log por cada CPU
 * @param id_cpu ID del CPU pasado como argumento a main
 * @return t_log* logger_cpu iniciado
 */
t_log* iniciar_logger_cpu(int id_cpu);

/** 
 * @brief Destruye el logger del kernel
 * 
 * Esta función no recibe parámetros. Destruye el logger del kernel
 * 
 */
void destruir_logger_cpu();

/**
 * @brief Convierte un tipo de instruccion (instruccion->tipo) a un string
 * @param tipo numero que representa en el enum de instruccion->tipo
 * @return Devuelve un string con el nombre de la instruccion
 */
char* instruccion_str(tipo_instruccion tipo);

/**
 * @brief Convierte los paraemtros de un tipo de intruccion a un string
 * @param instruccion Instruccion ya decodificada
 * @return char* string con los parametros de la instruccion
 */
char* parametros_str(instruccion_decodificada* instruccion);

#endif
