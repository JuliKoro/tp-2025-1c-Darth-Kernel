#include "memoria.h"
#include "memoria-admin.h"
#include "memoria-tablas.h"
#include "memoria-swap.h"
#include "memoria-marcos.h"

// Variables globales definidas en memoria.h
t_administrador_memoria* administrador_memoria = NULL;
t_bitarray* bitmap_swap = NULL;

 /**
  * @brief Inicializa el administrador de memoria.
  *
  * Reserva memoria para la estructura `t_administrador_memoria` y para la memoria principal.
  * Inicializa las listas de marcos libres y ocupados, los diccionarios de tablas de páginas
  * y métricas, y llama a las funciones de inicialización de marcos y SWAP.
  */
 void inicializar_administrador_memoria() {
    if (administrador_memoria == NULL) {
        administrador_memoria = malloc(sizeof(t_administrador_memoria));
        if (administrador_memoria == NULL) {
            log_error(logger_memoria, "Error al asignar memoria para el administrador de memoria.");
            exit(EXIT_FAILURE);
        }
        administrador_memoria->memoria_principal = malloc(memoria_configs.tammemoria);
        if (administrador_memoria->memoria_principal == NULL) {
            log_error(logger_memoria, "Error al asignar memoria principal de tamaño %d.", memoria_configs.tammemoria);
            free(administrador_memoria);
            exit(EXIT_FAILURE);
        }
        // Inicializa la memoria principal con ceros
        memset(administrador_memoria->memoria_principal, 0, memoria_configs.tammemoria);

        administrador_memoria->marcos_libres = list_create();
        administrador_memoria->marcos_ocupados = list_create();
        administrador_memoria->tablas_paginas = dictionary_create();
        administrador_memoria->metricas_por_proceso = dictionary_create();
        administrador_memoria->swap_file = NULL; // Se inicializa en inicializar_swap

        inicializar_marcos();
        inicializar_swap();
    }
}

/**
  * @brief Destruye todas las estructuras del administrador de memoria.
  *
  * Libera la memoria principal, las listas de marcos, los diccionarios
  * de tablas de páginas y métricas, y cierra el archivo SWAP.
  */
 void destruir_administrador_memoria() {
    if (administrador_memoria) {
        log_info(logger_memoria, "Destruyendo administrador de memoria...");

        // Liberar memoria principal
        if (administrador_memoria->memoria_principal) {
            free(administrador_memoria->memoria_principal);
            log_debug(logger_memoria, "Memoria principal liberada.");
        }

        // Destruir listas de marcos (los elementos son punteros a la memoria principal, no se liberan aquí)
        if (administrador_memoria->marcos_libres) {
            list_destroy(administrador_memoria->marcos_libres);
            log_debug(logger_memoria, "Lista de marcos libres destruida.");
        }
        if (administrador_memoria->marcos_ocupados) {
            list_destroy(administrador_memoria->marcos_ocupados);
            log_debug(logger_memoria, "Lista de marcos ocupados destruida.");
        }

        // Destruir tablas de páginas y sus elementos (libera marcos y posiciones SWAP)
        if (administrador_memoria->tablas_paginas) {
            dictionary_destroy_and_destroy_elements(administrador_memoria->tablas_paginas, destruir_tabla_paginas);
            log_debug(logger_memoria, "Diccionario de tablas de páginas destruido.");
        }

        // Destruir métricas por proceso
        if (administrador_memoria->metricas_por_proceso) {
            dictionary_destroy_and_destroy_elements(administrador_memoria->metricas_por_proceso, free);
            log_debug(logger_memoria, "Diccionario de métricas por proceso destruido.");
        }

        // Cerrar archivo SWAP
        if (administrador_memoria->swap_file) {
            fclose(administrador_memoria->swap_file);
            log_debug(logger_memoria, "Archivo SWAP cerrado.");
        }

        // Liberar el bitmap de SWAP
        if (bitmap_swap) {
            free(bitmap_swap->bitarray); // Liberar el buffer interno del bitarray
            bitarray_destroy(bitmap_swap);
            log_debug(logger_memoria, "Bitmap de SWAP destruido.");
        }

        // Liberar la estructura del administrador
        free(administrador_memoria);
        administrador_memoria = NULL;
        log_info(logger_memoria, "Administrador de memoria destruido completamente.");
    }
}

 /**
  * @brief Libera un marco de memoria.
  *
  * Elimina el marco de la lista de `marcos_ocupados` y lo añade a la lista de `marcos_libres`.
  * También actualiza la entrada de página que utilizaba este marco para marcarla como no presente.
  *
  * @param marco Puntero al marco de memoria a liberar.
  */
 void liberar_marco(void* marco) {
    // Eliminar el marco de la lista de marcos ocupados
    bool encontrado_en_ocupados = false;
    for (int i = 0; i < list_size(administrador_memoria->marcos_ocupados); i++) {
        if (list_get(administrador_memoria->marcos_ocupados, i) == marco) {
            list_remove(administrador_memoria->marcos_ocupados, i);
            encontrado_en_ocupados = true;
            break;
        }
    }
    if (!encontrado_en_ocupados) {
        log_warning(logger_memoria, "Intento de liberar marco %p que no estaba en la lista de ocupados.", marco);
    }

    // Buscar la entrada de página que usa este marco y limpiarla
    // Esta función auxiliar se ejecuta para cada tabla de páginas en el diccionario
    void _limpiar_entrada_por_marco(char* pid_key, void* tabla_void) {
        t_tabla_nivel* tabla = (t_tabla_nivel*)tabla_void;
        for (int j = 0; j < memoria_configs.entradasportabla; j++) {
            t_entrada_pagina* entrada = tabla->entradas[j];
            // Comprobar si la entrada está presente y si su marco coincide con el marco a liberar
            if (entrada->presente && (void*)((char*)administrador_memoria->memoria_principal +
                (entrada->marco * memoria_configs.tampagina)) == marco) {
                entrada->presente = false;
                entrada->marco = -1; // Marcar como marco no asignado
                log_debug(logger_memoria, "Entrada de página para PID %s, nivel %d, entrada %d, marco %p marcada como no presente.", pid_key, tabla->nivel_actual, j, marco);
                return; // Encontrado y limpiado, salir de la iteración de entradas
            }
        }
    }
    dictionary_iterator(administrador_memoria->tablas_paginas, _limpiar_entrada_por_marco);

    // Agregar el marco a la lista de marcos libres
    list_add(administrador_memoria->marcos_libres, marco);
    log_debug(logger_memoria, "Marco %p liberado y añadido a la lista de libres.", marco);
}

/**
  * @brief Busca la entrada de página y el PID asociado a un marco dado.
  *
  * Itera sobre todas las tablas de páginas de todos los procesos para encontrar
  * qué entrada de página está utilizando el `marco_victima` y a qué PID pertenece.
  *
  * @param marco_victima Puntero al marco de memoria a buscar.
  * @return Una estructura `t_marco_info` con el PID, la tabla y la entrada de página
  *         asociados al marco. Si no se encuentra, el PID será -1.
  */
 t_marco_info buscar_marco_en_tablas(void* marco_victima) {
    t_marco_info info = { .pid = -1, .tabla = NULL, .entrada = NULL };

    // Función auxiliar recursiva para buscar en tablas multinivel
    void _buscar_recursivo(t_tabla_nivel* current_tabla, char* current_pid_key) {
        if (info.pid != -1) return; // Si ya se encontró, salir

        for (int i = 0; i < memoria_configs.entradasportabla; i++) {
            t_entrada_pagina* entrada = current_tabla->entradas[i];

            if (current_tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
                // Si es una entrada de tabla de nivel intermedio
                // Asumimos que las tablas de nivel intermedio no ocupan marcos de datos directamente
                // y que su "marco" campo almacena la dirección de la siguiente tabla.
                // Si la entrada está "presente" (es decir, la tabla de nivel inferior existe)
                if (entrada->presente) {
                    t_tabla_nivel* next_tabla = (t_tabla_nivel*)(intptr_t)entrada->marco;
                    if (next_tabla) {
                        _buscar_recursivo(next_tabla, current_pid_key);
                        if (info.pid != -1) return; // Si se encontró en un nivel inferior, propagar
                    }
                }
            } else {
                // Último nivel: es una página de datos
                if (entrada->presente &&
                    (void*)((char*)administrador_memoria->memoria_principal +
                           (entrada->marco * memoria_configs.tampagina)) == marco_victima) {
                    info.pid = atoi(current_pid_key);
                    info.tabla = current_tabla;
                    info.entrada = entrada;
                    return; // Encontrado
                }
            }
        }
    }

    // Iterar sobre todas las tablas de páginas de nivel 0 (las principales de cada proceso)
    void _iterar_diccionario(char* pid_key, void* tabla_void) {
        if (info.pid != -1) return; // Si ya se encontró, salir

        t_tabla_nivel* tabla_nivel_0 = (t_tabla_nivel*)tabla_void;
        _buscar_recursivo(tabla_nivel_0, pid_key);
    }
    dictionary_iterator(administrador_memoria->tablas_paginas, _iterar_diccionario);

    return info;
}

 /**
  * @brief Obtiene la primera posición libre en el archivo SWAP.
  *
  * Busca el primer bit en 0 en el `bitmap_swap`, lo marca como 1 (ocupado)
  * y devuelve su índice.
  *
  * @return Índice de la posición libre, o -1 si no hay espacio.
  */
 int obtener_posicion_libre_swap() {
    for (int i = 0; i < bitmap_swap->size; i++) {
        if (!bitarray_test_bit(bitmap_swap, i)) {
            bitarray_set_bit(bitmap_swap, i);
            log_debug(logger_memoria, "Posición SWAP libre obtenida: %d", i);
            return i;
        }
    }
    log_warning(logger_memoria, "No hay espacio libre en SWAP.");
    return -1; // No hay espacio libre
}

/**
 * @brief Libera una posición en el archivo SWAP.
 *
 * Marca el bit correspondiente a la `posicion` en el `bitmap_swap` como 0 (libre).
 *
 * @param posicion Índice de la posición a liberar.
 */
void liberar_posicion_swap(int posicion) {
    if (posicion >= 0 && posicion < bitmap_swap->size) {
        bitarray_clean_bit(bitmap_swap, posicion);
        log_debug(logger_memoria, "Posición SWAP %d liberada.", posicion);
    } else {
        log_warning(logger_memoria, "Intento de liberar posición SWAP inválida: %d", posicion);
    }
}

/**
 * @brief Escribe una página de datos en el archivo SWAP.
 *
 * Escribe el contenido de `pagina` en la `posicion` especificada dentro
 * del archivo SWAP.
 *
 * @param posicion Índice de la posición en SWAP donde escribir.
 * @param pagina Puntero a los datos de la página a escribir.
 */
void escribir_pagina_swap(int posicion, void* pagina) {
    if (administrador_memoria->swap_file == NULL) {
        log_error(logger_memoria, "Archivo SWAP no abierto. No se puede escribir.");
        return;
    }
    if (posicion < 0 || posicion * memoria_configs.tampagina >= (long)bitmap_swap->size * memoria_configs.tampagina) {
        log_error(logger_memoria, "Posición SWAP %d fuera de los límites del archivo SWAP.", posicion);
        return;
    }

    fseek(administrador_memoria->swap_file, (long)posicion * memoria_configs.tampagina, SEEK_SET);
    fwrite(pagina, memoria_configs.tampagina, 1, administrador_memoria->swap_file);
    fflush(administrador_memoria->swap_file); // Asegura que los datos se escriban en disco
    log_debug(logger_memoria, "Página escrita en SWAP en posición %d.", posicion);
}

/**
 * @brief Lee una página de datos desde el archivo SWAP.
 *
 * Lee el contenido de la `posicion` especificada en el archivo SWAP
 * y lo copia a `destino`.
 *
 * @param posicion Índice de la posición en SWAP desde donde leer.
 * @param destino Puntero al buffer donde se copiarán los datos leídos.
 */
void leer_pagina_swap(int posicion, void* destino) {
    if (administrador_memoria->swap_file == NULL) {
        log_error(logger_memoria, "Archivo SWAP no abierto. No se puede leer.");
        return;
    }
    if (posicion < 0 || posicion * memoria_configs.tampagina >= (long)bitmap_swap->size * memoria_configs.tampagina) {
        log_error(logger_memoria, "Posición SWAP %d fuera de los límites del archivo SWAP.", posicion);
        return;
    }

    fseek(administrador_memoria->swap_file, (long)posicion * memoria_configs.tampagina, SEEK_SET);
    fread(destino, memoria_configs.tampagina, 1, administrador_memoria->swap_file);
    log_debug(logger_memoria, "Página leída de SWAP desde posición %d.", posicion);
}