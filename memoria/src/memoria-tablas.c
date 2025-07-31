#include "memoria.h"
#include "memoria-tablas.h"
#include "memoria-swap.h"
#include "memoria-procesos.h"
#include "memoria-operaciones.h"
#include "memoria-marcos.h"

/**
  * @brief Crea una nueva tabla de páginas para un nivel específico.
  *
  * Asigna memoria para la estructura `t_tabla_nivel` y para sus entradas.
  * Inicializa cada entrada como no presente, no modificada, sin marco asignado
  * y sin posición en SWAP. Si no es el último nivel, crea recursivamente
  * la tabla de páginas del siguiente nivel y almacena su dirección en el campo `marco`.
  *
  * @param nivel Nivel de la tabla de páginas a crear (0 para el primer nivel).
  * @return Puntero a la tabla de páginas creada.
  */
 t_tabla_nivel* crear_tabla_nivel(int nivel) {
    t_tabla_nivel* tabla = malloc(sizeof(t_tabla_nivel));
    if (tabla == NULL) {
        log_error(logger_memoria, "Error al asignar memoria para tabla de nivel %d.", nivel);
        exit(EXIT_FAILURE);
    }

    tabla->nivel_actual = nivel;
    tabla->entradas_ocupadas = 0;
    tabla->entradas = calloc(memoria_configs.entradasportabla, sizeof(t_entrada_pagina*));
    if (tabla->entradas == NULL) {
        log_error(logger_memoria, "Error al asignar memoria para entradas de tabla de nivel %d.", nivel);
        free(tabla);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < memoria_configs.entradasportabla; i++) {
        tabla->entradas[i] = malloc(sizeof(t_entrada_pagina));
        if (tabla->entradas[i] == NULL) {
            log_error(logger_memoria, "Error al asignar entrada %d en tabla de nivel %d", i, nivel);
            for (int j = 0; j < i; j++) free(tabla->entradas[j]);
            free(tabla->entradas);
            free(tabla);
            exit(EXIT_FAILURE);
        }

        tabla->entradas[i]->presente = false;
        tabla->entradas[i]->modificado = false;
        tabla->entradas[i]->marco = -1;
        tabla->entradas[i]->posicion_swap = -1;

        if (nivel < memoria_configs.cantidadniveles - 1) {
            t_tabla_nivel* subtabla = crear_tabla_nivel(nivel + 1);
            tabla->entradas[i]->marco = (intptr_t)subtabla;
            tabla->entradas[i]->presente = true;
            log_debug(logger_memoria, "Tabla de nivel %d, entrada %d: Creada tabla de nivel %d en dirección %p.", nivel, i, nivel + 1, subtabla);
        }
    }

    log_debug(logger_memoria, "Tabla de nivel %d creada en dirección %p.", nivel, tabla);
    return tabla;
}



/**
  * @brief Destruye una tabla de páginas y sus sub-tablas recursivamente.
  *
  * Libera la memoria de todas las entradas de la tabla y de la propia tabla.
  * Si una entrada apunta a una página de datos presente, libera el marco asociado.
  * Si una entrada tiene una posición en SWAP, la libera.
  * Si es una tabla de nivel intermedio, llama recursivamente a `destruir_tabla_paginas`
  * para las tablas de nivel inferior.
  *
  * @param tabla_void Puntero a la tabla de páginas a destruir (se castea a `t_tabla_nivel*`).
  */
 void destruir_tabla_paginas(void* tabla_void) {
    if (!tabla_void) return;

    t_tabla_nivel* tabla = (t_tabla_nivel*)tabla_void;
    log_debug(logger_memoria, "Destruyendo tabla de nivel %d en dirección %p.", tabla->nivel_actual, tabla);

    for (int i = 0; i < memoria_configs.entradasportabla; i++) {
        t_entrada_pagina* entrada = tabla->entradas[i];
        if (!entrada) continue;

        if (tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
            if (entrada->presente && entrada->marco != -1) {
                t_tabla_nivel* subtabla = (t_tabla_nivel*)(intptr_t)entrada->marco;

                // Validación adicional: evitar destruir punteros locos
                if (subtabla != NULL && subtabla != tabla) {
                    destruir_tabla_paginas(subtabla);
                    entrada->marco = -1;
                }
            }
        } else {
            if (entrada->presente && entrada->marco != -1) {
                void* marco_ptr = (char*)administrador_memoria->memoria_principal + (entrada->marco * memoria_configs.tampagina);
                liberar_marco(marco_ptr);
                log_debug(logger_memoria, "Marco %d liberado en nivel %d, entrada %d", entrada->marco, tabla->nivel_actual, i);
            }
        }

        if (entrada->posicion_swap != -1) {
            liberar_posicion_swap(entrada->posicion_swap);
            log_debug(logger_memoria, "Posición SWAP %d liberada en nivel %d, entrada %d", entrada->posicion_swap, tabla->nivel_actual, i);
        }

        free(entrada);
        tabla->entradas[i] = NULL;
    }

    free(tabla->entradas);
    free(tabla);
}



/**
  * @brief Maneja un fallo de página.
  *
  * Obtiene un marco libre (posiblemente desalojando otro), carga la página
  * desde SWAP si existe, o la inicializa con ceros si es nueva.
  * Actualiza la entrada de página para marcarla como presente y asignarle el marco.
  *
  * @param pid PID del proceso que sufrió el fallo de página.
  * @param entrada Puntero a la entrada de página que causó el fallo.
  * @param desplazamiento Desplazamiento dentro de la página.
  * @return Dirección física resultante del marco asignado más el desplazamiento,
  *         o -1 si no se pudo manejar el fallo (no hay marcos libres).
  */
 int manejar_fallo_pagina(int pid, t_entrada_pagina* entrada) {
    log_info(logger_memoria, "TLB MISS para PID %d. Asignando marco.", pid);

    void* nuevo_marco = obtener_marco_libre();
    if (!nuevo_marco) {
        log_error(logger_memoria, "No hay marcos libres para PID %d.", pid);
        return -1; // No hay marcos disponibles
    }

    if (entrada->posicion_swap != -1) {
        // Página en SWAP: cargarla a memoria principal
        leer_pagina_swap(entrada->posicion_swap, nuevo_marco);
        aplicar_retardo_swap();
        actualizar_metricas(pid, SUBIDA_MEMORIA); // Métrica subida desde SWAP
        liberar_posicion_swap(entrada->posicion_swap);
        entrada->posicion_swap = -1;
        log_info(logger_memoria, "PID %d: Página cargada desde SWAP a marco %ld.",
                 pid, ((char*)nuevo_marco - (char*)administrador_memoria->memoria_principal) / memoria_configs.tampagina);
    } else {
        // Página nueva: inicializar marco con ceros
        memset(nuevo_marco, 0, memoria_configs.tampagina);
        log_info(logger_memoria, "PID %d: Nueva página asignada en marco %ld.",
                 pid, ((char*)nuevo_marco - (char*)administrador_memoria->memoria_principal) / memoria_configs.tampagina);
    }

    // Actualizar entrada de tabla de páginas
    entrada->presente = true;
    entrada->marco = ((char*)nuevo_marco - (char*)administrador_memoria->memoria_principal) / memoria_configs.tampagina;
    entrada->modificado = false;

    log_debug(logger_memoria, "PID %d: fallo de página resuelto, marco asignado: %d.", pid, entrada->marco);

    // Devuelve número de marco, CPU calculará dirección física sumando desplazamiento
    return entrada->marco;
}


int32_t obtener_marco_de_memoria(uint32_t numero_pagina, uint32_t* entradas_niveles, uint32_t pid) {
    char pid_str[16];
    sprintf(pid_str, "%d", pid);

    t_tabla_nivel* tabla_actual = dictionary_get(administrador_memoria->tablas_paginas, pid_str);
    if (tabla_actual == NULL) {
        log_error(logger_memoria, "PID %d: Tabla de páginas raíz no encontrada.", pid);
        return -1;
    }

    aplicar_retardo_memoria(); // Cada acceso cuenta como acceso a tabla
    actualizar_metricas(pid, ACCESO_TABLA_PAGINA);

    for (int i = 0; i < memoria_configs.cantidadniveles - 1; i++) {
        uint32_t entrada_idx = entradas_niveles[i];
        if (entrada_idx >= memoria_configs.entradasportabla) {
            log_error(logger_memoria, "PID %d: Índice de entrada inválido (%d) en nivel %d", pid, entrada_idx, i);
            return -1;
        }

        t_entrada_pagina* entrada = tabla_actual->entradas[entrada_idx];
        if (!entrada || !entrada->presente) {
            log_error(logger_memoria, "PID %d: Entrada no presente en nivel %d, índice %d", pid, i, entrada_idx);
            return -1;
        }

        tabla_actual = (t_tabla_nivel*)(intptr_t)entrada->marco;

        aplicar_retardo_memoria();
        actualizar_metricas(pid, ACCESO_TABLA_PAGINA);
    }

    // Llegamos al último nivel
    uint32_t entrada_final = entradas_niveles[memoria_configs.cantidadniveles - 1];
    if (entrada_final >= memoria_configs.entradasportabla) {
        log_error(logger_memoria, "PID %d: Índice de entrada final inválido: %d", pid, entrada_final);
        return -1;
    }

    t_entrada_pagina* entrada_final_ptr = tabla_actual->entradas[entrada_final];
    if (!entrada_final_ptr) {
        log_error(logger_memoria, "PID %d: Entrada final NULL", pid);
        return -1;
    }

    if (!entrada_final_ptr->presente) {
        // Fallo de página → traer desde SWAP o inicializar
        int marco = manejar_fallo_pagina(pid, entrada_final_ptr);
        if (marco == -1) {
            log_error(logger_memoria, "PID %d: Fallo al manejar fallo de página.", pid);
            return -1;
        }
        return marco;
    }

    return entrada_final_ptr->marco;
}

