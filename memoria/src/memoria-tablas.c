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
    tabla->entradas_ocupadas = 0; // Inicialmente ninguna entrada está en uso
    tabla->entradas = malloc(memoria_configs.entradasportabla * sizeof(t_entrada_pagina*));
    if (tabla->entradas == NULL) {
        log_error(logger_memoria, "Error al asignar memoria para entradas de tabla de nivel %d.", nivel);
        // Liberar lo ya asignado antes de salir
        // El error estaba aquí: 'i' no estaba en el ámbito de este 'if'
        // Se debe usar un contador local o un goto para limpiar.
        // La forma más sencilla es usar un contador local para la limpieza.
        // O, mejor aún, refactorizar para que la limpieza se haga en un punto único.
        // Para este caso, asumimos que el 'i' del bucle principal es el que se quiere usar.
        // El error es un poco extraño, ya que 'i' debería ser visible.
        // Vamos a usar un bucle for con 'k' para evitar cualquier ambigüedad.
        for (int k = 0; k < memoria_configs.entradasportabla; k++) { // <-- Corrección aquí
            if (tabla->entradas[k] != NULL) { // Solo liberar si ya se asignó
                free(tabla->entradas[k]);
            }
        }
        free(tabla->entradas);
        free(tabla);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < memoria_configs.entradasportabla; i++) {
        tabla->entradas[i] = malloc(sizeof(t_entrada_pagina));
        if (tabla->entradas[i] == NULL) {
            log_error(logger_memoria, "Error al asignar memoria para entrada %d de tabla de nivel %d.", i, nivel);
            // Liberar lo ya asignado antes de salir
            for (int j = 0; j < i; j++) free(tabla->entradas[j]); // <-- Aquí 'i' es el contador del bucle principal
            free(tabla->entradas);
            free(tabla);
            exit(EXIT_FAILURE);
        }
        tabla->entradas[i]->presente = false;
        tabla->entradas[i]->modificado = false;
        tabla->entradas[i]->marco = -1; // -1 indica que no hay marco asignado
        tabla->entradas[i]->posicion_swap = -1; // -1 indica que no está en SWAP

        if (nivel < memoria_configs.cantidadniveles - 1) {
            // Si no es el último nivel, la entrada apunta a otra tabla de nivel
            // Se crea la tabla de nivel inferior y se almacena su dirección como un entero.
            // Esto asume que las tablas de nivel intermedio no ocupan marcos de datos,
            // sino que son estructuras de control en el espacio de Kernel.
            t_tabla_nivel* siguiente_tabla = crear_tabla_nivel(nivel + 1);
            tabla->entradas[i]->marco = (intptr_t)siguiente_tabla; // Almacenar la dirección como intptr_t
            tabla->entradas[i]->presente = true; // La tabla de nivel inferior "existe"
            log_debug(logger_memoria, "Tabla de nivel %d, entrada %d: Creada tabla de nivel %d en dirección %p.", nivel, i, nivel + 1, siguiente_tabla);
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
    t_tabla_nivel* tabla = (t_tabla_nivel*)tabla_void;
    if (tabla == NULL) return;

    log_debug(logger_memoria, "Destruyendo tabla de nivel %d en dirección %p.", tabla->nivel_actual, tabla);

    for (int i = 0; i < memoria_configs.entradasportabla; i++) {
        t_entrada_pagina* entrada = tabla->entradas[i];
        if (entrada == NULL) continue;

        if (tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
            // Si es una tabla de nivel intermedio, destruir la tabla de nivel inferior
            // Solo si la entrada estaba "presente" (es decir, la tabla de nivel inferior fue creada)
            if (entrada->presente) {
                t_tabla_nivel* siguiente_tabla = (t_tabla_nivel*)(intptr_t)entrada->marco;
                if (siguiente_tabla != NULL) {
                    destruir_tabla_paginas(siguiente_tabla);
                }
            }
        } else {
            // Último nivel: es una página de datos
            if (entrada->presente && entrada->marco != -1) {
                // Si la página está en memoria principal, liberar el marco
                void* marco_a_liberar = (char*)administrador_memoria->memoria_principal + (entrada->marco * memoria_configs.tampagina);
                liberar_marco(marco_a_liberar);
                log_debug(logger_memoria, "Marco %d liberado al destruir tabla de nivel %d, entrada %d.", entrada->marco, tabla->nivel_actual, i);
            }
        }

        // Liberar la posición en SWAP si la página estaba allí
        if (entrada->posicion_swap != -1) {
            liberar_posicion_swap(entrada->posicion_swap);
            log_debug(logger_memoria, "Posición SWAP %d liberada al destruir tabla de nivel %d, entrada %d.", entrada->posicion_swap, tabla->nivel_actual, i);
        }
        free(entrada); // Liberar la estructura de la entrada
    }
    free(tabla->entradas); // Liberar el array de punteros a entradas
    free(tabla);           // Liberar la estructura de la tabla
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
 int manejar_fallo_pagina(int pid, t_entrada_pagina* entrada, int desplazamiento) {
    log_info(logger_memoria, "Fallo de página detectado para PID %d. Intentando asignar marco.", pid);

    void* nuevo_marco = obtener_marco_libre();
    if (!nuevo_marco) {
        log_error(logger_memoria, "No hay marcos libres para manejar fallo de página para PID %d. Operación fallida.", pid);
        return -1; // No hay marcos disponibles
    }

    if (entrada->posicion_swap != -1) {
        // La página estaba en SWAP, cargarla a memoria principal
        leer_pagina_swap(entrada->posicion_swap, nuevo_marco);
        aplicar_retardo_swap(); // Aplicar retardo por acceso a SWAP
        actualizar_metricas(pid, SUBIDA_MEMORIA); // Métrica de subida a Memoria Principal
        liberar_posicion_swap(entrada->posicion_swap); // Liberar la posición en SWAP
        entrada->posicion_swap = -1; // Marcar como no en SWAP
        // Corrección: Usar %ld para el resultado de la resta de punteros
        log_info(logger_memoria, "Página de PID %d cargada desde SWAP a marco %ld.", pid, ((char*)nuevo_marco - (char*)administrador_memoria->memoria_principal) / memoria_configs.tampagina);
    } else {
        // Es una página nueva, inicializar el marco con ceros
        memset(nuevo_marco, 0, memoria_configs.tampagina);
        // Corrección: Usar %ld para el resultado de la resta de punteros
        log_info(logger_memoria, "Nueva página asignada para PID %d en marco %ld.", pid, ((char*)nuevo_marco - (char*)administrador_memoria->memoria_principal) / memoria_configs.tampagina);
    }

    entrada->presente = true;
    entrada->marco = ((char*)nuevo_marco - (char*)administrador_memoria->memoria_principal) / memoria_configs.tampagina;
    entrada->modificado = false; // Se asume que al cargarla o crearla no está modificada

    log_debug(logger_memoria, "Fallo de página manejado para PID %d. Marco asignado: %d.", pid, entrada->marco);
    return entrada->marco * memoria_configs.tampagina + desplazamiento;
}

 /**
  * @brief Traduce una dirección lógica a una dirección física.
  *
  * Recorre las tablas de páginas del proceso (`pid`) nivel por nivel,
  * calculando los índices de entrada en cada tabla. Si se encuentra un fallo
  * de página, lo maneja.
  *
  * @param pid PID del proceso.
  * @param direccion_logica Dirección lógica a traducir.
  * @return Dirección física resultante, o -1 si hay un error (ej. tabla no encontrada, fallo de página no manejado).
  */
 int traducir_direccion(int pid, int direccion_logica, t_entrada_pagina** entrada_out) {
    char pid_str[16];
    sprintf(pid_str, "%d", pid);

// VALIDACIÓN ADICIONAL
    if (direccion_logica < 0) {
        log_error(logger_memoria, "PID %d: Dirección lógica negativa (%d)", pid, direccion_logica);
        return -1;
    }

    t_tabla_nivel* tabla_nivel_0 = dictionary_get(administrador_memoria->tablas_paginas, pid_str);
    aplicar_retardo_memoria(); // Retardo por acceso a tabla de páginas de nivel 0

    if (!tabla_nivel_0) {
        log_error(logger_memoria, "Tabla de páginas no encontrada para PID: %d. No se puede traducir dirección lógica %d.", pid, direccion_logica);
        return -1;
    }

    int temp_dir = direccion_logica;
    int desplazamiento = temp_dir % memoria_configs.tampagina;
    int numero_pagina_logica = temp_dir / memoria_configs.tampagina;

    t_tabla_nivel* tabla_actual = tabla_nivel_0;
    t_entrada_pagina* entrada_final = NULL; // La entrada que contiene el marco de la página de datos

    for (int nivel = 0; nivel < memoria_configs.cantidadniveles; nivel++) {
        int indice_entrada = numero_pagina_logica % memoria_configs.entradasportabla;
        numero_pagina_logica /= memoria_configs.entradasportabla; // Para el siguiente nivel

        if (indice_entrada >= memoria_configs.entradasportabla || indice_entrada < 0) {
            log_error(logger_memoria, "PID %d: Índice de entrada de tabla de páginas fuera de rango. Nivel: %d, Índice: %d.", pid, nivel, indice_entrada);
            return -1;
        }

        t_entrada_pagina* entrada_actual = tabla_actual->entradas[indice_entrada];
        actualizar_metricas(pid, ACCESO_TABLA_PAGINA); // Métrica por acceso a tabla de páginas
        aplicar_retardo_memoria(); // Retardo por acceso a cada nivel de tabla de páginas

        if (!entrada_actual->presente) {
            // Fallo de página: la página (o la tabla de nivel inferior) no está en memoria principal
            log_info(logger_memoria, "PID %d: Fallo de página en nivel %d, índice %d. La entrada no está presente.", pid, nivel, indice_entrada);
            int direccion_fisica_resultante = manejar_fallo_pagina(pid, entrada_actual, desplazamiento);
            if (direccion_fisica_resultante == -1) {
                log_error(logger_memoria, "PID %d: No se pudo manejar el fallo de página. Traducción fallida.", pid);
            }
            return direccion_fisica_resultante;
        }

        if (nivel < memoria_configs.cantidadniveles - 1) {
            // Si no es el último nivel, la entrada apunta a la siguiente tabla de nivel
            // El campo 'marco' de la entrada de nivel intermedio contiene la dirección física de la siguiente tabla.
            // Se asume que las tablas de nivel intermedio se cargan en memoria principal como cualquier otra página.
            //void* direccion_fisica_siguiente_tabla = (char*)administrador_memoria->memoria_principal + (entrada_actual->marco * memoria_configs.tampagina);
            //tabla_actual = (t_tabla_nivel*)direccion_fisica_siguiente_tabla;
            
            tabla_actual = (t_tabla_nivel*)(intptr_t)entrada_actual->marco;
           
            log_debug(logger_memoria, "PID %d: Acceso a tabla de nivel %d. Siguiente tabla en marco %d.", pid, nivel, entrada_actual->marco);
        } else {
            // Último nivel: esta entrada contiene el marco de la página de datos
            entrada_final = entrada_actual;
            log_debug(logger_memoria, "PID %d: Último nivel alcanzado. Marco de datos: %d.", pid, entrada_final->marco);
        }
    }

    if (entrada_final == NULL || !entrada_final->presente) {
        // Esto no debería ocurrir si el manejo de fallo de página es correcto y se llega al último nivel.
        log_error(logger_memoria, "PID %d: Error lógico en traducción de dirección %d. Entrada final no encontrada o no presente.", pid, direccion_logica);
        return -1;
    }

    // La dirección física es el inicio del marco más el desplazamiento
    int direccion_fisica = entrada_final->marco * memoria_configs.tampagina + desplazamiento;
    log_debug(logger_memoria, "PID %d: Dirección lógica %d traducida a física %d (marco %d, desplazamiento %d).", pid, direccion_logica, direccion_fisica, entrada_final->marco, desplazamiento);
    
    if (entrada_final != NULL && entrada_out != NULL) {
        *entrada_out = entrada_final;
    }
    
    return direccion_fisica;
}

