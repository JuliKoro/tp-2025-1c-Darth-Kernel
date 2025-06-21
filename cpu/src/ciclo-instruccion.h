#ifndef CICLO_INSTRUCCION_H_
#define CICLO_INSTRUCCION_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <commons/string.h>

#include "cpu-configs.h"
#include "cpu-log.h"

#include <utils/configs.h>
#include <utils/sockets.h>
#include <utils/serializacion.h>

/**
 * @enum tipo_instruccion
 * @brief Tipos de instrucciones soportadas por la CPU
 */
typedef enum {
    NOOP,           // No operation (instrucción vacía)
    WRITE,          // Escritura en memoria
    READ,           // Lectura de memoria
    GOTO,           // Salto incondicional
    IO,             // Operación de entrada/salida
    INIT_PROC,      // Inicialización de proceso
    DUMP_MEMORY,    // Volcado de memoria
    EXIT,           // Finalización de proceso
    INSTRUCCION_DESCONOCIDA // Tipo reservado para errores
} tipo_instruccion;

/**
 * @struct instruccion_decodificada
 * @brief Estructura que contiene una instrucción decodificada
 * 
 * Utiliza un campo 'tipo' para discriminar qué otros campos son válidos.
 * Todos los campos de datos son opcionales según el tipo de instrucción.
 */
typedef struct {
    tipo_instruccion tipo;    // Tipo de instrucción
    uint32_t direccion;         // Dirección de memoria (READ/WRITE)
    char* datos;                // Datos a escribir (WRITE)
    uint32_t tamanio;           // Tamaño de operación (READ/INIT_PROC)
    char* dispositivo_io;        // Nombre dispositivo (IO)
    uint32_t tiempo_io;         // Tiempo de operación (IO)
    char* archivo_proceso;      // Path del archivo (INIT_PROC)
    uint32_t pc_destino;        // Destino del salto (GOTO)
} instruccion_decodificada;

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
char* fetch(int pid, int pc, int socket_memoria);

/**
 * @brief Decodifica una instrucción en string a estructura
 * 
 * @param instruccion_str Cadena con la instrucción (ej: "WRITE 0x1A 'HOLA'")
 * @param pc_actual Program Counter actual (para logs)
 * @return instruccion_decodificada* Estructura decodificada. DEBE liberarse con destruir_instruccion()
 * 
 * @example 
 * instruccion_decodificada* instr = decodificar_instruccion("WRITE 0x1A 42", 15);
 */
instruccion_decodificada* decodificar_instruccion(char* instruccion_str, uint32_t pc_actual);

/**
 * @brief Libera los recursos de una instrucción decodificada
 * 
 * @param instruccion Puntero a la instrucción a liberar
 */
void destruir_instruccion(instruccion_decodificada* instruccion);

#endif