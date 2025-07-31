/**
 * @brief Cabecera principal del módulo de Memoria.
 *
 * Define las estructuras de datos fundamentales para la gestión de memoria,
 * incluyendo el administrador de memoria, las tablas de páginas, las entradas de página,
 * y las métricas por proceso. También declara los prototipos de las funciones
 * relacionadas con la inicialización, gestión de marcos, SWAP, traducción de direcciones,
 * lectura/escritura en memoria, y finalización de procesos.
 */

 #ifndef MEMORIA_H_
 #define MEMORIA_H_
 
 #include <commons/collections/dictionary.h>
 #include <commons/collections/list.h>
 #include <commons/bitarray.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <stdbool.h>
 #include <string.h>
 #include <unistd.h> // Para usleep
 #include <time.h>   // Para timestamp en memory dump
 #include <inttypes.h> // Para intptr_t
 
 #include "memoria-conexiones.h"
 #include "memoria-configs.h"
 #include "memoria-log.h"
 #include "utils/serializacion.h" // Para t_pcb y op_code
 
 // Estructura para almacenar las métricas de cada proceso
 typedef struct {
     int pid;
     int accesos_tablas_paginas;
     int instrucciones_solicitadas;
     int bajadas_swap;
     int subidas_memoria;
     int lecturas_memoria;
     int escrituras_memoria;
 } t_metricas_por_proceso;
 
 // Estructura para una entrada en la tabla de páginas
 typedef struct {
     int marco; // Si es una página de datos, es el número de marco.
                // Si es una entrada de tabla de nivel intermedio, es la dirección física de la siguiente tabla.
     int posicion_swap; // Posición en el archivo SWAP (-1 si no está en SWAP)
     bool presente;     // Indica si la página está en memoria principal
     bool modificado;   // Indica si la página ha sido modificada desde que se cargó
 } t_entrada_pagina;
 
 // Estructura para una tabla de páginas de un nivel
 typedef struct {
     int nivel_actual;         // Nivel actual de la tabla (0 para el primer nivel)
     int entradas_ocupadas;    // Cantidad de entradas actualmente en uso (no estrictamente necesario para este TP, pero útil)
     t_entrada_pagina** entradas; // Array de punteros a entradas de página
 } t_tabla_nivel;
 
 // Estructura principal para la administración de memoria
 typedef struct {
     void* memoria_principal;        // Puntero al espacio contiguo de memoria principal
     t_list* marcos_libres;          // Lista de punteros a marcos de memoria libres
     t_list* marcos_ocupados;        // Lista de punteros a marcos de memoria ocupados (para FIFO)
     t_dictionary* tablas_paginas;   // Diccionario de tablas de páginas, indexado por PID (char* -> t_tabla_nivel*)
     t_dictionary* metricas_por_proceso; // Diccionario de métricas, indexado por PID (char* -> t_metricas_por_proceso*)
     FILE* swap_file;                // Puntero al archivo de SWAP
 } t_administrador_memoria;
 
 // Estructura auxiliar para buscar información de un marco
 typedef struct {
     int pid;
     t_tabla_nivel* tabla;
     t_entrada_pagina* entrada;
 } t_marco_info;
 
 // Variables globales
 extern t_administrador_memoria* administrador_memoria;
 extern t_bitarray* bitmap_swap; // Bitmap para la gestión del espacio en SWAP
 
 // Definir tipos de operaciones para métricas (para la función actualizar_metricas)
 typedef enum {
     ACCESO_TABLA_PAGINA,
     SOLICITUD_INSTRUCCION_MET, // Renombrado para evitar conflicto con op_code
     BAJADA_SWAP,
     SUBIDA_MEMORIA,
     LECTURA_MEMORIA_MET,       // Renombrado
     ESCRITURA_MEMORIA_MET      // Renombrado
 } tipo_operacion;
 
 
 
 #endif /* MEMORIA_H_ */
 
 