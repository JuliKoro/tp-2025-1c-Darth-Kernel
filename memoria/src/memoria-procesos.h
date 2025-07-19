/**
 * @brief Cabecera para la gestión de instrucciones de procesos.
 *
 * Define la estructura `t_proceso` para almacenar las instrucciones de un PID.
 * Declara el diccionario global `procesos_en_memoria` y los prototipos de las
 * funciones para leer archivos de pseudocódigo, cargar procesos, obtener
 * instrucciones específicas y destruir estructuras de procesos.
 */

 #ifndef INSTRUCCIONES_H_ // Se cambió de INSTRUCCIONES a INSTRUCCIONES_H_ para seguir convención
 #define INSTRUCCIONES_H_
 
 // Commons
 #include <commons/collections/dictionary.h>
 #include <commons/string.h>
 #include <commons/log.h>
 
 // Sistema
 #include <dirent.h> // Para manejo de directorios
 #include <stdio.h>  // Para manejo de archivos (fopen, fgets, fclose)
 #include <stdlib.h> // Para malloc, free, exit
 #include <string.h> // Para strdup, strcspn, strcmp
 
 // Locales
 #include "memoria-configs.h" // Para acceder a memoria_configs.pathinstrucciones
 #include "memoria-log.h"     // Para usar el logger_memoria
 #include "memoria.h"         // Para t_tabla_nivel, t_metricas_por_proceso, etc.

  // Estructura para representar un proceso y sus instrucciones
  typedef struct {
    int pid;
    char** instrucciones;       // Array de strings con las instrucciones
    int cantidad_instrucciones; // Número total de instrucciones
    // t_list* paginas_asignadas; // No es necesario aquí, se gestiona en t_tabla_nivel
} t_proceso;

// Diccionario global para almacenar los procesos cargados en memoria (sus instrucciones)
extern t_dictionary* procesos_en_memoria;

/**
 * @brief Función mock para obtener espacio libre (solo de prueba).
 * @return Un valor fijo de espacio libre (1024 bytes).
 */
int obtener_espacio_libre_mock();

/**
 * @brief Lee un archivo de pseudocódigo y lo carga en una estructura `t_proceso`.
 *
 * Abre el archivo especificado por `filepath`, lee cada línea como una instrucción,
 * y las almacena en un array de strings dentro de una nueva estructura `t_proceso`.
 *
 * @param filepath Ruta completa al archivo de pseudocódigo.
 * @param pid PID del proceso al que pertenecen las instrucciones.
 * @return Puntero a la estructura `t_proceso` creada, o NULL si no se pudo abrir el archivo.
 */
t_proceso* leer_archivo_de_proceso(const char* filepath, int pid);

/**
 * @brief Destruye una estructura `t_proceso` y libera sus recursos.
 *
 * Libera la memoria de las instrucciones y de la propia estructura `t_proceso`.
 * También muestra las métricas finales del proceso antes de liberarlas.
 * Esta función está diseñada para ser usada como `element_destroyer` en `dictionary_destroy_and_destroy_elements`.
 *
 * @param proceso_void Puntero a la estructura `t_proceso` a destruir (se castea internamente).
 */
bool destruir_proceso(void* proceso_void);

/**
 * @brief Carga un proceso en memoria.
 *
 * Lee el archivo de pseudocódigo del proceso, crea la estructura `t_proceso`
 * y la almacena en el diccionario global `procesos_en_memoria`.
 * También crea la tabla de páginas de nivel 0 para el nuevo proceso.
 *
 * @param pid PID del proceso a cargar.
 * @param nombre_archivo Nombre del archivo de pseudocódigo (ej. "123.txt").
 * @return 0 si el proceso se cargó exitosamente, -1 en caso de error.
 */
int cargar_proceso(int pid, const char* nombre_archivo);

/**
 * @brief Carga todos los procesos encontrados en el directorio de instrucciones.
 *
 * Itera sobre los archivos en `PATH_INSTRUCCIONES`, asume que el nombre del archivo
 * es el PID del proceso, y llama a `cargar_proceso` para cada uno.
 */
void cargar_procesos_en_memoria();

/**
 * @brief Muestra las instrucciones de un proceso por consola.
 *
 * Función auxiliar para depuración, imprime el PID y cada instrucción del proceso.
 * Diseñada para ser usada con `dictionary_iterator`.
 *
 * @param key Clave del diccionario (PID como string).
 * @param value Puntero a la estructura `t_proceso`.
 */
void mostrar_proceso(char* key, void* value);

/**
 * @brief Obtiene una instrucción específica de un proceso.
 *
 * Busca el proceso por su PID y devuelve la instrucción en la posición `pc` (Program Counter).
 *
 * @param pid PID del proceso.
 * @param pc Program Counter (índice de la instrucción, 1-based).
 * @return Puntero a la cadena de caracteres de la instrucción, o NULL si no se encuentra el proceso o el PC está fuera de rango.
 */
const char* obtener_instruccion(int pid, int pc);

/**
 * @brief Cabecera para la gestión de procesos.
 *
 * Declara las funciones relacionadas con la suspensión, des-suspensión,
 * finalización de procesos, y generación de memory dumps.
 */

/**
 * @brief Actualiza las métricas de un proceso.
 *
 * Incrementa el contador correspondiente al `tipo_operacion` para el PID dado.
 * Si el proceso no tiene métricas registradas, se inicializan.
 *
 * @param pid PID del proceso.
 * @param tipo_operacion Tipo de operación realizada (ej. ACCESO_TABLA_PAGINA, LECTURA_MEMORIA_MET).
 */
void actualizar_metricas(int pid, int tipo_operacion);

/**
 * @brief Suspende un proceso.
 *
 * Recorre todas las páginas de datos del proceso. Si una página está presente
 * en memoria principal, la escribe en SWAP (si está modificada o no tiene posición
 * asignada), la marca como no presente y libera el marco.
 *
 * @param pid PID del proceso a suspender.
 */
int suspender_proceso(int pid);

/**
 * @brief Des-suspende un proceso.
 *
 * Recorre todas las páginas de datos del proceso. Si una página no está presente
 * pero tiene una posición en SWAP, obtiene un marco libre, carga la página desde
 * SWAP, libera la posición en SWAP y actualiza la entrada de página.
 *
 * @param pid PID del proceso a des-suspender.
 */
void desuspender_proceso(int pid);

/**
 * @brief Finaliza un proceso y libera todos sus recursos.
 *
 * Libera los recursos de memoria principal y SWAP asociados al proceso
 * utilizando `destruir_tabla_paginas`. Muestra las métricas finales del proceso
 * y elimina el proceso de la lista de procesos en memoria.
 *
 * @param pid PID del proceso a finalizar.
 */
int finalizar_proceso(int pid);

/**
 * @brief Realiza un "memory dump" de la memoria principal de un proceso.
 *
 * Crea un archivo con el nombre `<PID>-<TIMESTAMP>.dmp` en el `DUMP_PATH`
 * configurado. Escribe en este archivo el contenido de todas las páginas
 * del proceso que están actualmente en memoria principal.
 *
 * @param pid PID del proceso para el cual se realizará el dump.
 */
void realizar_memory_dump(int pid);

void aplicar_retardo_swap();

t_tabla_nivel* crear_tabla_nivel(int nivel);
void liberar_posicion_swap(int posicion);

#endif /* MEMORIA_PROCESOS_H */
