#ifndef MEMORIA_H_
#define MEMORIA_H_


#include "memoria-conexiones.h"
#include "instrucciones.h"
#include "utils/serializacion.h"

typedef struct {
    int pid;
    int accesos_tablas_paginas;
    int instrucciones_solicitadas;
    int bajadas_swap;
    int subidas_memoria;
    int lecturas_memoria;
    int escrituras_memoria;
} t_metricas_proceso;

typedef struct {
    void* memoria_principal; // Espacio contiguo de memoria
    t_list* marcos_libres;  // Lista de marcos libres
    t_dictionary* tablas_paginas; // Diccionario de tablas de páginas por proceso
    t_dictionary* metricas_proceso; // Diccionario de métricas por proceso
    FILE* swap_file; // Archivo de swap
} t_administrador_memoria;

extern t_dictionary* metricas_proceso;
extern t_administrador_memoria* administrador_memoria;

typedef struct {
    int marco; // -1 si no está en memoria
    int posicion_swap; // -1 si no está en swap
} t_entrada_pagina;

typedef struct {
    t_list* entradas; // Lista de t_entrada_pagina
} t_tabla_paginas;

// Prototipos de funciones
void inicializar_administrador_memoria();
void inicializar_marcos();
void inicializar_swap();
void* obtener_marco_libre();
void liberar_marco(void* marco);
void actualizar_metricas(int pid, int tipo_operacion);
void realizar_memory_dump(int pid);

#endif