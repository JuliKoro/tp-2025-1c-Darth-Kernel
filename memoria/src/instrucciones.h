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

typedef struct {
    int pid;
    char** instrucciones;
    int cantidad_instrucciones;
} t_proceso;

int obtener_espacio_libre_mock();

t_proceso* leer_archivo_de_proceso(const char* filepath, int pid);

void destruir_proceso(void* proceso_void);

void cargar_proceso(int pid, const char* nombre_archivo);

void cargar_procesos_en_memoria();

void mostrar_proceso(char* key, void* value);

const char* obtener_instruccion(int pid, int pc);

#endif