#ifndef INSTRUCCIONES
#define INSTRUCCIONES

// Commons
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <commons/log.h>

// Sistema
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Locales
#include "memoria-configs.h"
#include "memoria-log.h"

extern t_dictionary* procesos_en_memoria;

/**
 * @brief TODO
 * 
 * @param pid: TODO
 * @param instrucciones: TODO
 * @param cantidad_instrucciones: TODO
 * @param paginas_asignadas: TODO
 */
typedef struct {
    int pid;
    char** instrucciones;
    int cantidad_instrucciones;
    t_list* paginas_asignadas; // Lista de páginas asignadas a este proceso
} t_proceso;

/**
 * @brief TODO
 * 
 * @return TODO
 */
int obtener_espacio_libre_mock();

/**
 * @brief TODO
 * 
 * @param filepath: TODO
 * @param pid: TODO
 * @return TODO
 */
t_proceso* leer_archivo_de_proceso(const char* filepath, int pid);

/**
 * @brief TODO
 * 
 * @param proceso_void: TODO
 */
void destruir_proceso(void* proceso_void);

/**
 * @brief TODO
 * 
 * @param pid: TODO
 * @param nombre_archivo: TODO
 * @return TODO
 */
int cargar_proceso(int pid, const char* nombre_archivo);

/**
 * @brief TODO
 * 
 */
void cargar_procesos_en_memoria();

/**
 * @brief TODO
 * 
 * @param key: TODO
 * @param value: TODO
 */
void mostrar_proceso(char* key, void* value);

/**
 * @brief TODO
 * 
 * @param pid: TODO
 * @param pc: TODO
 * @return TODO
 */
const char* obtener_instruccion(int pid, int pc);

/**
 * @brief TODO
 * 
 * @param socket_memoria: TODO
 * @return TODO
 */
void* recibir_peticiones_kernel(void* socket_memoria);

#endif