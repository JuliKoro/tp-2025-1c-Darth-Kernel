#include "memoria.h"
#include "memoria-procesos.h"
#include "memoria-admin.h"
#include "memoria-marcos.h"
#include "memoria-tablas.h"
#include <stdint.h>   // intptr_t
#include <time.h>     // time, strftime
#include <limits.h>


// El PATH_MAX es una constante del sistema que define el tamaño máximo de una ruta de archivo, y en algunos entornos no viene definida por defecto.
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


 // Variable global definida en instrucciones.h
 t_dictionary* procesos_en_memoria;

  /**
  * @brief Función mock para obtener espacio libre (solo de prueba).
  * @return Un valor fijo de espacio libre (1024 bytes).
  */
 int obtener_espacio_libre_mock() {
    return 1024; // Retorna 1KB como espacio libre (solo de prueba, lo está simulando)
 }
  /**
  * @brief Muestra las instrucciones de un proceso por consola.
  *
  * Función auxiliar para depuración, imprime el PID y cada instrucción del proceso.
  * Diseñada para ser usada con `dictionary_iterator`.
  *
  * @param key Clave del diccionario (PID como string).
  * @param value Puntero a la estructura `t_proceso`.
  */
 void mostrar_proceso(char* key, void* value) {
    t_proceso* proceso = (t_proceso*)value;
    if (proceso == NULL) return;

    printf("PID: %d\n", proceso->pid);
    for (int i = 0; i < proceso->cantidad_instrucciones; i++) {
        printf("  - %s\n", proceso->instrucciones[i]);
    }
}

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
 t_proceso* leer_archivo_de_proceso(const char* filepath, int pid) {
    FILE* archivo = fopen(filepath, "r");
    if (!archivo) {
        log_error(logger_memoria, "No se pudo abrir el archivo de pseudocódigo para PID %d en ruta: %s", pid, filepath);
        return NULL;
    }

    // Se asigna un tamaño inicial, se puede ajustar dinámicamente si es necesario
    // Para este TP, un tamaño fijo de 100 instrucciones debería ser suficiente.
    char** instrucciones = malloc(sizeof(char*) * 100);
    if (instrucciones == NULL) {
        log_error(logger_memoria, "Error al asignar memoria para las instrucciones del PID %d.", pid);
        fclose(archivo);
        return NULL;
    }
    int cantidad = 0;
    char linea[256]; // Buffer para leer cada línea

    while (fgets(linea, sizeof(linea), archivo)) {
        // Remueve el salto de línea si existe
        linea[strcspn(linea, "\n")] = 0;
        // Copia segura de la línea a la lista de instrucciones
        instrucciones[cantidad] = strdup(linea);
        if (instrucciones[cantidad] == NULL) {
            log_error(logger_memoria, "Error al duplicar instrucción para PID %d.", pid);
            // Liberar lo ya asignado antes de salir
            for (int i = 0; i < cantidad; i++) free(instrucciones[i]);
            free(instrucciones);
            fclose(archivo);
            return NULL;
        }
        cantidad++;
        // Si se supera el tamaño inicial, se podría reallocar aquí
        if (cantidad >= 100) {
            log_warning(logger_memoria, "Se superó el límite inicial de 100 instrucciones para PID %d. Considerar realloc.", pid);
            // Para este TP, se asume que 100 es suficiente o se maneja el error.
            break;
        }
    }

    fclose(archivo);

    t_proceso* proceso = malloc(sizeof(t_proceso));
    if (proceso == NULL) {
        log_error(logger_memoria, "Error al asignar memoria para la estructura t_proceso del PID %d.", pid);
        for (int i = 0; i < cantidad; i++) free(instrucciones[i]);
        free(instrucciones);
        return NULL;
    }
    proceso->pid = pid;
    proceso->instrucciones = instrucciones;
    proceso->cantidad_instrucciones = cantidad;

    log_debug(logger_memoria, "Archivo de pseudocódigo para PID %d leído. %d instrucciones cargadas.", pid, cantidad);
    return proceso;
}



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
int cargar_proceso(int pid, const char* nombre_archivo) {
    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", memoria_configs.pathinstrucciones, nombre_archivo);

    t_proceso* proceso = leer_archivo_de_proceso(fullpath, pid);

    if (proceso != NULL) {
        // Asegurarse de que el diccionario esté inicializado
        if (procesos_en_memoria == NULL) {
            procesos_en_memoria = dictionary_create();
            log_warning(logger_memoria, "procesos_en_memoria no estaba inicializado. Se crea uno nuevo.");
        } else {
            log_warning(logger_memoria, "El diccionario de procesos en memoria ya estaba NULL.");
        }

        char* pid_key = string_itoa(pid);
        if (pid_key == NULL) {
            log_error(logger_memoria, "Error al asignar memoria para pid_key para PID %d.", pid);
            destruir_proceso(proceso);
            return -1;
        }

        t_tabla_nivel* tabla_nivel_0 = crear_tabla_nivel(0);
        if (tabla_nivel_0 == NULL) {
            log_error(logger_memoria, "Error al crear tabla de páginas para PID %d.", pid);
            free(pid_key);
            destruir_proceso(proceso);
            return -1;
        }

        dictionary_put(administrador_memoria->tablas_paginas, pid_key, tabla_nivel_0);
        log_debug(logger_memoria, "Tabla de páginas de nivel 0 creada y asociada a PID %d.", pid);

        dictionary_put(procesos_en_memoria, pid_key, proceso);

        int tamanio_proceso = proceso->cantidad_instrucciones * memoria_configs.tampagina;
        log_info(logger_memoria, "## PID: %d - Proceso Creado - Tamaño: %d", pid, tamanio_proceso); //LOG OBLIGATORIO
        return 0;
    }

    log_error(logger_memoria, "No se pudo cargar el proceso PID %d desde archivo %s.", pid, nombre_archivo);
    return -1;
}


 /**
  * @brief Obtiene una instrucción específica de un proceso.
  *
  * Busca el proceso por su PID y devuelve la instrucción en la posición `pc` (Program Counter).
  *
  * @param pid PID del proceso.
  * @param pc Program Counter (índice de la instrucción, 1-based).
  * @return Puntero a la cadena de caracteres de la instrucción, o NULL si no se encuentra el proceso o el PC está fuera de rango.
  */
 const char* obtener_instruccion(int pid, int pc) {
    char pid_key[16]; // Suficiente para un int y el null terminator
    sprintf(pid_key, "%d", pid);

    t_proceso* proceso = dictionary_get(procesos_en_memoria, pid_key);
    // El PC es 1-based, el array de instrucciones es 0-based.
    // Se verifica que pc sea mayor que 0 y no exceda la cantidad de instrucciones.
    if (proceso != NULL && pc >= 0 && pc <= proceso->cantidad_instrucciones) {
        const char* instruccion = proceso->instrucciones[pc];
        // Log obligatorio: Obtener instrucción
        log_info(logger_memoria, "## PID: %d - Obtener instrucción: %d - Instrucción: %s", pid, pc, instruccion); // LOG OBLIGATORIO
        return instruccion;
    }

    log_warning(logger_memoria, "PID %d: PC %d fuera de rango (0 a %d).", pid, pc, proceso->cantidad_instrucciones - 1);
    return NULL;
}

 /**
  * @brief Actualiza las métricas de un proceso.
  *
  * Incrementa el contador correspondiente al `tipo_operacion` para el PID dado.
  * Si el proceso no tiene métricas registradas, se inicializan.
  *
  * @param pid PID del proceso.
  * @param tipo_operacion Tipo de operación realizada (ej. ACCESO_TABLA_PAGINA, LECTURA_MEMORIA_MET).
  */
 void actualizar_metricas(int pid, int tipo_operacion) {
    char pid_str[16];
    sprintf(pid_str, "%d", pid);

    t_metricas_por_proceso* metricas = dictionary_get(administrador_memoria->metricas_por_proceso, pid_str);
    if (!metricas) {
        // Si no existen métricas para este PID, crearlas e inicializarlas
        metricas = malloc(sizeof(t_metricas_por_proceso));
        if (metricas == NULL) {
            log_error(logger_memoria, "Error al asignar memoria para métricas del PID %d.", pid);
            return;
        }
        *metricas = (t_metricas_por_proceso){0}; // Inicializar todos los campos a 0
        metricas->pid = pid;
        // Se duplica la clave para que el diccionario sea dueño de la memoria de la clave
        dictionary_put(administrador_memoria->metricas_por_proceso, strdup(pid_str), metricas);
        log_debug(logger_memoria, "Métricas inicializadas para PID %d.", pid);
    }

    switch (tipo_operacion) {
        case ACCESO_TABLA_PAGINA:
            metricas->accesos_tablas_paginas++;
            log_debug(logger_memoria, "PID %d: Acceso a tabla de páginas. Total: %d", pid, metricas->accesos_tablas_paginas);
            break;
        case LECTURA_MEMORIA_MET:
            metricas->lecturas_memoria++;
            log_debug(logger_memoria, "PID %d: Lectura de memoria. Total: %d", pid, metricas->lecturas_memoria);
            break;
        case ESCRITURA_MEMORIA_MET:
            metricas->escrituras_memoria++;
            log_debug(logger_memoria, "PID %d: Escritura de memoria. Total: %d", pid, metricas->escrituras_memoria);
            break;
        case BAJADA_SWAP:
            metricas->bajadas_swap++;
            log_debug(logger_memoria, "PID %d: Bajada a SWAP. Total: %d", pid, metricas->bajadas_swap);
            break;
        case SUBIDA_MEMORIA:
            metricas->subidas_memoria++;
            log_debug(logger_memoria, "PID %d: Subida a Memoria Principal. Total: %d", pid, metricas->subidas_memoria);
            break;
        case SOLICITUD_INSTRUCCION_MET:
            metricas->instrucciones_solicitadas++;
            log_debug(logger_memoria, "PID %d: Solicitud de instrucción. Total: %d", pid, metricas->instrucciones_solicitadas);
            break;
        default:
            log_warning(logger_memoria, "Tipo de operación de métrica desconocido: %d para PID %d.", tipo_operacion, pid);
            break;
    }
}

 /**
  * @brief Suspende un proceso.
  *
  * Recorre todas las páginas de datos del proceso. Si una página está presente
  * en memoria principal, la escribe en SWAP (si está modificada o no tiene posición
  * asignada), la marca como no presente y libera el marco.
  *
  * @param pid PID del proceso a suspender.
  */
// Función recursiva para suspender páginas 
static void suspender_paginas_recursivo(t_tabla_nivel *current_tabla, int pid) {
    for (int i = 0; i < memoria_configs.entradasportabla; i++) {
        t_entrada_pagina *entrada = current_tabla->entradas[i];
        if (!entrada) continue;

        if (current_tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
            if (entrada->presente) {
                t_tabla_nivel *next_tabla = entrada->subnivel;
                suspender_paginas_recursivo(next_tabla, pid);
            }
            continue;
        }

        /* último nivel – página de datos */
        if (!entrada->presente) continue;

        void *marco_ptr = (char *)administrador_memoria->memoria_principal +
                          (entrada->marco * memoria_configs.tampagina);

        if (entrada->posicion_swap == -1) {
            entrada->posicion_swap = obtener_posicion_libre_swap();
            if (entrada->posicion_swap == -1) {
                log_error(logger_memoria,
                          "No hay espacio en SWAP para suspender PID %d. Página %d no se swapea.",
                          pid, i);
                continue;
            }
        }

        escribir_pagina_swap(entrada->posicion_swap, marco_ptr);
        entrada->modificado = false;
        aplicar_retardo_swap();
        actualizar_metricas(pid, BAJADA_SWAP);

        log_debug(logger_memoria, "PID %d: Página %d swapeada a posición %d.", pid, i,
                  entrada->posicion_swap);

        entrada->presente = false;
        liberar_marco(marco_ptr);
        entrada->marco = -1;
    }
}

int suspender_proceso(int pid) {
    char pid_str[16];
    sprintf(pid_str, "%d", pid);
    int resultado = 0;

    t_tabla_nivel *tabla = dictionary_get(administrador_memoria->tablas_paginas, pid_str);
    if (!tabla) {
        log_warning(logger_memoria,
                    "Intento de suspender PID %d, pero no se encontró su tabla de páginas.",
                    pid);
        return -1;
    }

    suspender_paginas_recursivo(tabla, pid);
    log_debug(logger_memoria, "## PID: %d - Proceso Suspendido", pid);
    return resultado;
}

 /**
  * @brief Des-suspende un proceso.
  *
  * Recorre todas las páginas de datos del proceso. Si una página no está presente
  * pero tiene una posición en SWAP, obtiene un marco libre, carga la página desde
  * SWAP, libera la posición en SWAP y actualiza la entrada de página.
  *
  * @param pid PID del proceso a des-suspender.
  */
// Función recursiva para desuspender páginas
static void desuspender_paginas_recursivo(t_tabla_nivel *current_tabla, int pid) {
    for (int i = 0; i < memoria_configs.entradasportabla; i++) {
        t_entrada_pagina *entrada = current_tabla->entradas[i];
        if (!entrada) continue;

        if (current_tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
            if (entrada->presente) {
                t_tabla_nivel *next_tabla = entrada->subnivel;
                desuspender_paginas_recursivo(next_tabla, pid);
            }
            continue;
        }

        /* último nivel – página de datos */
        if (entrada->presente || entrada->posicion_swap == -1) continue;

        void *marco = obtener_marco_libre();
        if (!marco) {
            log_error(logger_memoria,
                      "No hay marcos libres para desuspender PID %d. Proceso incompleto.", pid);
            return; // abortar – se requerirá re‑intento futuro
        }

        leer_pagina_swap(entrada->posicion_swap, marco);
        aplicar_retardo_swap();
        liberar_posicion_swap(entrada->posicion_swap);
        entrada->posicion_swap = -1;
        entrada->presente = true;
        entrada->marco = ((char *)marco - (char *)administrador_memoria->memoria_principal) /
                         memoria_configs.tampagina;
        actualizar_metricas(pid, SUBIDA_MEMORIA);
        log_debug(logger_memoria, "PID %d: Página %d cargada en marco %d.", pid, i,
                  entrada->marco);
    }
}

void desuspender_proceso(int pid) {
    char pid_str[16];
    sprintf(pid_str, "%d", pid);

    t_tabla_nivel *tabla = dictionary_get(administrador_memoria->tablas_paginas, pid_str);
    if (!tabla) {
        log_warning(logger_memoria,
                    "Intento de desuspender PID %d, pero no se encontró su tabla de páginas.",
                    pid);
        return;
    }

    desuspender_paginas_recursivo(tabla, pid);
    log_debug(logger_memoria, "## PID: %d - Proceso Desuspendido", pid);
}

/**
  * @brief Destruye una estructura `t_proceso` y libera sus recursos.
  * 
  * Libera la memoria de las instrucciones y de la propia estructura `t_proceso`.
  * Esta función está diseñada para ser usada como `element_destroyer` en diccionarios.
  *
  * @param proceso_void Puntero a la estructura `t_proceso` a destruir.
  */
 void destruir_proceso(void* proceso_void) {
    t_proceso* proceso = (t_proceso*)proceso_void;
    if (proceso == NULL) return;

    // Liberar instrucciones del proceso
    if (proceso->instrucciones) {
        for (int i = 0; i < proceso->cantidad_instrucciones; i++) {
            free(proceso->instrucciones[i]);
        }
        free(proceso->instrucciones);
    }
    free(proceso);
}


/**
  * @brief Finaliza un proceso y libera todos sus recursos.
  * 
  * Libera tablas de páginas, registra métricas, y destruye el proceso.
  *
  * @param pid PID del proceso a finalizar.
  */
int finalizar_proceso(int pid) {
    char pid_str[16];
    sprintf(pid_str, "%d", pid);

    int resultado = 0;

    // Liberar tabla de páginas
    t_tabla_nivel *tabla = dictionary_remove(administrador_memoria->tablas_paginas, pid_str);
    if (tabla) {
        destruir_tabla_paginas(tabla);
    } else {
        resultado = -1;
    }

    // Registrar métricas y liberarlas
    t_metricas_por_proceso *metricas = dictionary_remove(administrador_memoria->metricas_por_proceso, pid_str);
    if (metricas) {
        log_info(logger_memoria,
                 "## PID: %d - Proceso Destruido - Métricas - Acc.T.Pag: %d; Inst.Sol.: %d; SWAP: %d; Mem.Prin.: %d; Lec.Mem.: %d; Esc.Mem.: %d",
                 pid, 
                 metricas->accesos_tablas_paginas, 
                 metricas->instrucciones_solicitadas,
                 metricas->bajadas_swap, 
                 metricas->subidas_memoria, 
                 metricas->lecturas_memoria,
                 metricas->escrituras_memoria); // LOG OBLIGATORIO
        free(metricas);
    } else {
        resultado = -1;
    }

    // Liberar la estructura del proceso
    t_proceso *proc = dictionary_remove(procesos_en_memoria, pid_str);
    if (proc) {
        destruir_proceso(proc);
    } else {
        resultado = -1;
    }
    return resultado;
}

 /**
  * @brief Realiza un "memory dump" de la memoria principal de un proceso.
  *
  * Crea un archivo con el nombre `<PID>-<TIMESTAMP>.dmp` en el `DUMP_PATH`
  * configurado. Escribe en este archivo el contenido de todas las páginas
  * del proceso que están actualmente en memoria principal.
  *
  * @param pid PID del proceso para el cual se realizará el dump.
  */

  static void dump_paginas_recursivo(t_tabla_nivel *current_tabla, int pid, FILE *f) {

    if (!current_tabla) {
        log_error(logger_memoria, "Tabla actual NULL al intentar realizar dump para PID %d.", pid);
        return;
    }

    log_debug(logger_memoria, "Dump PID %d: Entrando a tabla nivel %d en dirección %p", pid, current_tabla->nivel_actual, (void*)current_tabla);

    for (int i = 0; i < memoria_configs.entradasportabla; i++) {
        t_entrada_pagina *entrada = current_tabla->entradas[i];
        
        if (!entrada) {
            log_warning(logger_memoria, "Entrada %d en nivel %d es NULL para PID %d.", i, current_tabla->nivel_actual, pid);
            continue;
        }

        // Si NO estamos en el último nivel, bajamos al subnivel
        if (current_tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
            if (entrada->presente && entrada->subnivel != NULL) {
                log_debug(logger_memoria, "Dump PID %d: Nivel %d, entrada %d -> Subnivel en dirección %p", pid, current_tabla->nivel_actual, i, (void*)entrada->subnivel);
                dump_paginas_recursivo(entrada->subnivel, pid, f);
            }
            continue;
        }

        // Si estamos en el último nivel y la página está presente, escribimos su contenido
        if (!entrada->presente) continue;

        void *data = (char *)administrador_memoria->memoria_principal +
                     (entrada->marco * memoria_configs.tampagina);

        log_debug(logger_memoria, "Dump PID %d: Escribiendo página de datos desde marco %d (Dirección: %p)", pid, entrada->marco, data);
        fwrite(data, 1, memoria_configs.tampagina, f);
    }
}



int realizar_memory_dump(int pid) {
    char pid_str[16];
    sprintf(pid_str, "%d", pid);

    char* ruta_directorio = memoria_configs.dumppath;
    mkdir(ruta_directorio, 0777);

    t_tabla_nivel *tabla = dictionary_get(administrador_memoria->tablas_paginas, pid_str);
    if (!tabla) {
        log_warning(logger_memoria, "PID %d – dump fallido: sin tabla de páginas", pid);
        return -1;
    }

    /* construir nombre de archivo <PID>-<timestamp>.dmp en DUMP_PATH */
    char timestamp[32];
    time_t now = time(NULL);
    strftime(timestamp, sizeof timestamp, "%Y%m%d-%H%M%S", localtime(&now));

    char filepath[PATH_MAX];
    snprintf(filepath, sizeof filepath, "%s/%d-%s.dmp", memoria_configs.dumppath, pid,
             timestamp);

    FILE *f = fopen(filepath, "wb");
    if (!f) {
        log_error(logger_memoria, "No se pudo crear el dump en %s", filepath);
        return -1;
    }

    dump_paginas_recursivo(tabla, pid, f);
    fclose(f);

    log_debug(logger_memoria, "## PID: %d - Memory‑Dump generado en %s", pid, filepath);
    return 0;
}