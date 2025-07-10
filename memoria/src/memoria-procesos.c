#include "memoria.h"
#include "memoria-procesos.h"
#include "memoria-admin.h"
#include "memoria-marcos.h"


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
  * @brief Destruye una estructura `t_proceso` y libera sus recursos.
  *
  * Libera la memoria de las instrucciones y de la propia estructura `t_proceso`.
  * También muestra las métricas finales del proceso antes de liberarlas.
  * Esta función está diseñada para ser usada como `element_destroyer` en `dictionary_destroy_and_destroy_elements`.
  *
  * @param proceso_void Puntero a la estructura `t_proceso` a destruir (se castea internamente).
  */
 void destruir_proceso(void* proceso_void) {
    t_proceso* proceso = (t_proceso*)proceso_void;
    if (proceso == NULL) return;

    char pid_str[16];
    sprintf(pid_str, "%d", proceso->pid);

    // Mostrar métricas antes de que sean liberadas por `finalizar_proceso`
    // NOTA: La lógica de liberación de tablas de páginas y métricas se ha movido
    // a `finalizar_proceso` en `memoria.c` para centralizar la liberación de recursos
    // del proceso. Esta función `destruir_proceso` ahora solo se encarga de liberar
    // la estructura `t_proceso` y sus instrucciones.
    // Si `destruir_proceso` es llamada por `dictionary_destroy_and_destroy_elements`
    // al final del `main`, las métricas ya habrán sido mostradas y liberadas por
    // `finalizar_proceso` cuando el proceso terminó su ejecución.
    // Por lo tanto, el bloque de métricas aquí es redundante si `finalizar_proceso`
    // ya se encarga de ello. Se mantiene el log de debug para claridad.

    log_debug(logger_memoria, "Liberando instrucciones para PID %d.", proceso->pid);
    if (proceso->instrucciones) {
        for (int i = 0; i < proceso->cantidad_instrucciones; i++) {
            if (proceso->instrucciones[i]) {
                free(proceso->instrucciones[i]);
            }
        }
        free(proceso->instrucciones);
    }
    free(proceso);
    log_debug(logger_memoria, "Estructura t_proceso para PID %d liberada.", proceso->pid);
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
    // Construye la ruta completa al archivo de pseudocódigo
    snprintf(fullpath, sizeof(fullpath), "%s/%s", memoria_configs.pathinstrucciones, nombre_archivo);

    t_proceso* proceso = leer_archivo_de_proceso(fullpath, pid);

    if (proceso != NULL) {
        char* pid_key = string_itoa(pid); // Convierte el PID a string para usarlo como clave
        if (pid_key == NULL) {
            log_error(logger_memoria, "Error al asignar memoria para pid_key para PID %d.", pid);
            destruir_proceso(proceso); // Liberar el proceso si no se puede almacenar
            return -1;
        }

        // Crear tabla de páginas para el nuevo proceso
        // Se crea la tabla de nivel 0, que recursivamente creará los niveles inferiores.
        t_tabla_nivel* tabla_nivel_0 = crear_tabla_nivel(0);
        if (tabla_nivel_0 == NULL) {
            log_error(logger_memoria, "Error al crear tabla de páginas para PID %d.", pid);
            free(pid_key);
            destruir_proceso(proceso);
            return -1;
        }
        // Almacenar la tabla de páginas en el diccionario de tablas de páginas del administrador de memoria
        dictionary_put(administrador_memoria->tablas_paginas, pid_key, tabla_nivel_0);
        log_debug(logger_memoria, "Tabla de páginas de nivel 0 creada y asociada a PID %d.", pid);

        // Almacenar la estructura de proceso (instrucciones) en el diccionario global
        dictionary_put(procesos_en_memoria, pid_key, proceso);

       // Calcular tamaño real del proceso (páginas * tamaño_página)
       int tamanio_proceso = proceso->cantidad_instrucciones * memoria_configs.tampagina;

        // Log obligatorio: Creación de Proceso
        log_info(logger_memoria, "## PID: %d - Proceso Creado - Tamaño: %d", pid, tamanio_proceso); // El tamaño real se determinaría por las páginas asignadas
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
    if (proceso != NULL && pc > 0 && pc <= proceso->cantidad_instrucciones) {
        const char* instruccion = proceso->instrucciones[pc - 1];
        // Log obligatorio: Obtener instrucción
        log_info(logger_memoria, "## PID: %d - Obtener instrucción: %d - Instrucción: %s", pid, pc, instruccion);
        return instruccion;
    }

    log_warning(logger_memoria, "No se encontró el proceso con PID %d o PC %d fuera de rango (total instrucciones: %d).",
               pid, pc, (proceso != NULL ? proceso->cantidad_instrucciones : 0));
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
 void suspender_proceso(int pid) {
    char pid_str[16];
    sprintf(pid_str, "%d", pid);

    t_tabla_nivel* tabla = dictionary_get(administrador_memoria->tablas_paginas, pid_str);
    if (!tabla) {
        log_warning(logger_memoria, "Intento de suspender PID %d, pero no se encontró su tabla de páginas.", pid);
        return;
    }

    // Función auxiliar recursiva corregida
    void _suspender_paginas_recursivo(t_tabla_nivel* current_tabla, int current_pid) {
        for (int i = 0; i < memoria_configs.entradasportabla; i++) {
            t_entrada_pagina* entrada = current_tabla->entradas[i];
            if (entrada == NULL) continue;

            if (current_tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
                if (entrada->presente) {
                    t_tabla_nivel* next_tabla = (t_tabla_nivel*)(intptr_t)entrada->marco;
                    if (next_tabla) {
                        _suspender_paginas_recursivo(next_tabla, current_pid);
                    }
                }
            } else {
                if (entrada->presente) {
                    void* marco = (char*)administrador_memoria->memoria_principal +
                                  (entrada->marco * memoria_configs.tampagina);

                    if (entrada->posicion_swap == -1) {
                        entrada->posicion_swap = obtener_posicion_libre_swap();
                        if (entrada->posicion_swap == -1) {
                            log_error(logger_memoria, 
                                "No hay espacio en SWAP para suspender PID %d. Suspensión incompleta.",
                                current_pid);
                            continue;
                        }
                    }

                    escribir_pagina_swap(entrada->posicion_swap, marco);
                    entrada->modificado = false;
                    aplicar_retardo_swap();
                    actualizar_metricas(current_pid, BAJADA_SWAP);

                    log_debug(logger_memoria, "PID %d: Página en marco %d swapeada a posición %d.", current_pid, entrada->marco, entrada->posicion_swap);

                    entrada->presente = false;
                    liberar_marco(marco);
                    entrada->marco = -1;
                    
                    log_debug(logger_memoria, "PID %d: Marco %p liberado.", current_pid, marco);
                }
            }
        }
    }
    
    _suspender_paginas_recursivo(tabla, pid);
    log_info(logger_memoria, "## PID: %d - Proceso Suspendido", pid);
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
 void desuspender_proceso(int pid) {
    char pid_str[16];
    sprintf(pid_str, "%d", pid);

    t_tabla_nivel* tabla = dictionary_get(administrador_memoria->tablas_paginas, pid_str);
    if (!tabla) {
        log_warning(logger_memoria, "Intento de desuspender PID %d, pero no se encontró su tabla de páginas.", pid);
        return;
    }

    // Función auxiliar recursiva para desuspender páginas en tablas multinivel
    void _desuspender_paginas_recursivo(t_tabla_nivel* current_tabla, int current_pid) {
        for (int i = 0; i < memoria_configs.entradasportabla; i++) {
            t_entrada_pagina* entrada = current_tabla->entradas[i];
            if (entrada == NULL) continue;

            if (current_tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
                // Si es una entrada de tabla de nivel intermedio, ir al siguiente nivel
                if (entrada->presente) { // Si la tabla de nivel inferior "existe"
                    t_tabla_nivel* next_tabla = (t_tabla_nivel*)(intptr_t)entrada->marco;
                    if (next_tabla) {
                        _desuspender_paginas_recursivo(next_tabla, current_pid);
                    }
                }
            } else {
                // Último nivel: es una página de datos
                if (!entrada->presente && entrada->posicion_swap != -1) {
                    // La página no está en memoria principal pero sí en SWAP
                    void* marco = obtener_marco_libre();
                    if (!marco) {
                        log_error(logger_memoria, "No hay marcos libres para desuspender PID %d. No se puede desuspender completamente.", current_pid);
                        return; // No se puede desuspender completamente
                    }

                    leer_pagina_swap(entrada->posicion_swap, marco);
                    aplicar_retardo_swap();
                    liberar_posicion_swap(entrada->posicion_swap); // Liberar espacio en SWAP
                    entrada->posicion_swap = -1; // Marcar como no en SWAP

                    entrada->presente = true;
                    entrada->marco = ((char*)marco - (char*)administrador_memoria->memoria_principal) / memoria_configs.tampagina;
                    actualizar_metricas(current_pid, SUBIDA_MEMORIA);
                    log_debug(logger_memoria, "PID %d: Página cargada de SWAP a marco %d.", current_pid, entrada->marco);
                }
            }
        }
    }
    _desuspender_paginas_recursivo(tabla, pid);
    log_info(logger_memoria, "## PID: %d - Proceso Desuspendido", pid);
}

 /**
  * @brief Finaliza un proceso y libera todos sus recursos.
  *
  * Libera los recursos de memoria principal y SWAP asociados al proceso
  * utilizando `destruir_tabla_paginas`. Muestra las métricas finales del proceso
  * y elimina el proceso de la lista de procesos en memoria.
  *
  * @param pid PID del proceso a finalizar.
  */
 void finalizar_proceso(int pid) {
    char pid_str[16];
    sprintf(pid_str, "%d", pid);

    // 1. Liberar recursos de memoria principal y SWAP
    // La función destruir_tabla_paginas ya se encarga de liberar marcos y posiciones SWAP
    // Se usa dictionary_remove_and_destroy para que el diccionario libere la clave y el valor
    t_tabla_nivel* tabla_removida = dictionary_remove(administrador_memoria->tablas_paginas, pid_str);
    if (tabla_removida) {
        destruir_tabla_paginas(tabla_removida);
        log_debug(logger_memoria, "PID %d: Tablas de páginas y recursos asociados liberados.", pid);
    } else {
        log_warning(logger_memoria, "PID %d: No se encontró la tabla de páginas para finalizar el proceso.", pid);
    }

    // 2. Mostrar métricas y liberarlas
    t_metricas_por_proceso* metricas = dictionary_remove(administrador_memoria->metricas_por_proceso, pid_str);
    if (metricas) {
        log_info(logger_memoria,
               "## PID: %d - Proceso Destruido - Métricas - Acc.T.Pag: %d; Inst.Sol.: %d; SWAP: %d; Mem.Prin.: %d; Lec.Mem.: %d; Esc.Mem.: %d",
               pid,
               metricas->accesos_tablas_paginas,
               metricas->instrucciones_solicitadas,
               metricas->bajadas_swap,
               metricas->subidas_memoria,
               metricas->lecturas_memoria,
               metricas->escrituras_memoria);
        free(metricas); // Liberar la estructura de métricas
        log_debug(logger_memoria, "PID %d: Métricas liberadas.", pid);
    } else {
        log_warning(logger_memoria, "PID %d: No se encontraron métricas para mostrar al finalizar el proceso.", pid);
    }

    // 3. Eliminar el proceso de la lista de procesos en memoria (instrucciones)
    // Se asume que `procesos_en_memoria` es un diccionario global que almacena `t_proceso*`.
    // La función `destruir_proceso` (definida en instrucciones.c) se encarga de liberar
    // la estructura `t_proceso` y sus instrucciones.
    // Aquí se llama directamente a `destruir_proceso` para liberar el `t_proceso*`
    // que se obtiene del diccionario `procesos_en_memoria`.
    t_proceso* proceso_a_destruir = dictionary_remove(procesos_en_memoria, pid_str);
    if (proceso_a_destruir) {
        // Liberar las instrucciones y la estructura del proceso
        for (int i = 0; i < proceso_a_destruir->cantidad_instrucciones; i++) {
            free(proceso_a_destruir->instrucciones[i]);
        }
        free(proceso_a_destruir->instrucciones);
        free(proceso_a_destruir);
        log_debug(logger_memoria, "PID %d: Estructura de proceso (instrucciones) liberada.", pid);
    } else {
        log_warning(logger_memoria, "PID %d: No se encontró la estructura de proceso (instrucciones) para liberar.", pid);
    }

    log_info(logger_memoria, "## PID: %d - Proceso Finalizado y recursos liberados.", pid);
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
 void realizar_memory_dump(int pid) {
    char timestamp[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", t); // Formato YYYYMMDDHHMMSS

    char path_dump[512];
    snprintf(path_dump, sizeof(path_dump), "%s/%d-%s.dmp",
             memoria_configs.dumppath, pid, timestamp);

    FILE* dump_file = fopen(path_dump, "wb");
    if (!dump_file) {
        log_error(logger_memoria, "Error al crear archivo de dump '%s' para PID %d. Verifique la ruta y permisos.", path_dump, pid);
        return;
    }

    char pid_str[16];
    sprintf(pid_str, "%d", pid);
    t_tabla_nivel* tabla_nivel_0 = dictionary_get(administrador_memoria->tablas_paginas, pid_str);

    if (tabla_nivel_0) {
        log_info(logger_memoria, "Iniciando Memory Dump para PID %d en '%s'.", pid, path_dump);

        // Función auxiliar recursiva para recorrer y dumpear páginas
        void _dumpear_paginas_recursivo(t_tabla_nivel* current_tabla) {
            for (int i = 0; i < memoria_configs.entradasportabla; i++) {
                t_entrada_pagina* entrada = current_tabla->entradas[i];
                if (entrada == NULL) continue;

                if (current_tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
                    // Si es una entrada de tabla de nivel intermedio, ir al siguiente nivel
                    if (entrada->presente) {
                        t_tabla_nivel* next_tabla = (t_tabla_nivel*)(intptr_t)entrada->marco;
                        if (next_tabla) {
                            _dumpear_paginas_recursivo(next_tabla);
                        }
                    }
                } else {
                    // Último nivel: es una página de datos
                    if (entrada->presente && entrada->marco != -1) {
                        void* marco = (char*)administrador_memoria->memoria_principal +
                                      (entrada->marco * memoria_configs.tampagina);
                        fwrite(marco, memoria_configs.tampagina, 1, dump_file);
                        log_debug(logger_memoria, "PID %d: Página en marco %d dumpeada.", pid, entrada->marco);
                    }
                }
            }
        }
        _dumpear_paginas_recursivo(tabla_nivel_0);
    } else {
        log_warning(logger_memoria, "No se encontró la tabla de páginas para PID %d. No se realizará el Memory Dump.", pid);
    }

    fclose(dump_file);
    log_info(logger_memoria, "## PID: %d - Memory Dump solicitado", pid);
}