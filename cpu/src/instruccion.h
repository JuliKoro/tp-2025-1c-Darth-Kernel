#ifndef INSTRUCCION_H_
#define INSTRUCCION_H_

#include <stdint.h>

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
    EXIT_INSTR,     // Finalización de proceso (nombrado asi por error de declaracion)
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
    uint32_t pid;             // PID al que pertenece la instruccion (para logs)
    tipo_instruccion tipo;    // Tipo de instrucción
    bool requiere_traduccion;   // Identificador para las instrucciones que requieren traducción
    uint32_t direccion;         // Dirección de memoria (READ/WRITE)
    char* datos;                // Datos a escribir (WRITE)
    uint32_t tamanio;           // Tamaño de operación (READ/INIT_PROC)
    char* dispositivo_io;        // Nombre dispositivo (IO)
    uint32_t tiempo_io;         // Tiempo de operación (IO)
    char* archivo_proceso;      // Path del archivo (INIT_PROC)
    uint32_t pc_destino;        // Destino del salto (GOTO)
} instruccion_decodificada;

#endif