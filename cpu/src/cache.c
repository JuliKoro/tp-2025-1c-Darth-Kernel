#include "cache.h"

cache_t* cache = NULL; // Declaracion de variable global

cache_t* crear_cache(int capacidad, char* algoritmo_reemplazo) {
    cache_t* cache = malloc(sizeof(cache_t));
    cache->entradas = malloc(sizeof(entrada_cache) * cpu_configs.entradascache);
    cache->capacidad = cpu_configs.entradascache;
    cache->tamanio = 0;
    cache->algoritmo_reemplazo = cpu_configs.reemplazocache;

    // Inicializacion de las entradas
    for (int i = 0; i < cpu_configs.entradascache; i++) {
        cache->entradas[i].uso = false;
        cache->entradas[i].modificado = false;
    }

    return cache;
}

void destruir_cache() {
    /*
    // Liberar el contenido de las entradas si es necesario
    for (int i = 0; i < cache->capacidad; i++) {
        // Si hay contenido que liberar, hazlo aquí
        free(cache->entradas[i].contenido); // Suponiendo que el contenido fue asignado dinámicamente
    }
    */
    free(cache->entradas);
    free(cache);
}

bool acceder_cache() {
    // Verificar si la caché está habilitada
    if (cache == NULL || cache->capacidad <= 0) {
        log_debug(logger_cpu, "Caché deshabilitada, accediendo directamente a memoria.");
        return false; // La caché está deshabilitada, no se puede acceder
    }
    return true;
}

bool acceder_pagina_cache(uint32_t pagina, uint32_t pid) {
    for (int i = 0; i < cache->capacidad; i++) {
        if (cache->entradas[i].pagina == pagina) {
            // Cache Hit
            cache->entradas[i].uso = true; // Marco como usada
            log_info(logger_cpu, "PID: %d - Cache Hit - Pagina: %d", pid, pagina);
            return true; // Página encontrada en caché
        }
    }
    // Cache Miss
    log_info(logger_cpu, "PID: %d - Cache Miss - Pagina: %d", pid, pagina);
    return false; // Página no encontrada en caché
}

void cargar_pagina_en_cache(uint32_t pagina, uint32_t pid, int socket_memoria) {
    // Obtengo la DF de la página
    uint32_t direccion_fisica = traducir_direccion_logica(pagina, pid, socket_memoria);
    
    // Creo y serializo la solicitud para cargar la página
    t_lectura_memoria solicitud;
    solicitud.pid = pid;
    solicitud.direccion_fisica = direccion_fisica;
    solicitud.tamanio = 0; // No es de interes para solicitar una pagina para la Cache

    t_buffer* buffer_solicitud = serializar_solicitud_pag(&solicitud);
    
    // Envio la solicitud a memoria
    t_paquete paquete_solicitud;
    paquete_solicitud.codigo_operacion = PAQUETE_SOLICITUD_PAG;
    paquete_solicitud.buffer = buffer_solicitud;

    if (enviar_paquete(socket_memoria, &paquete_solicitud) == -1) {
        log_error(logger_cpu, "Error al enviar solicitud de carga de página a Memoria.");
        return; // Manejo de error
    }

    // Recibo la respuesta de memoria
    t_paquete* paquete_respuesta = recibir_paquete(socket_memoria);
    if (paquete_respuesta->codigo_operacion != PAQUETE_PAG_CACHE) {
        log_error(logger_cpu, "Error al recibir la respuesta de Memoria para la carga de página.");
        return; // Manejo de error
    }

    t_contenido_pag* pagina_cache = deserializar_contenido_pagina(paquete_respuesta->buffer);
    
    // Agrego la página a la caché
    agregar_a_cache(pagina, pagina_cache->contenido, pid);

    // Liberar el paquete de respuesta y el contenido deserializado
    liberar_paquete(paquete_respuesta);
    free(pagina_cache);
}

void agregar_a_cache(uint32_t pagina, void* contenido, uint32_t pid) {
    if (cache->tamanio < cache->capacidad) {
        // Hay espacio en la caché
        cache->entradas[cache->tamanio].pagina = pagina;
        cache->entradas[cache->tamanio].contenido = contenido;
        cache->entradas[cache->tamanio].uso = true;
        cache->entradas[cache->tamanio].modificado = false; // Inicialmente no modificado
        cache->tamanio++;
    } else {
        // Caché llena, hay que reemplazar una página
        reemplazar_pagina(pagina, pid);
    }
    log_info(logger_cpu, "PID: %d - Cache Add - Pagina: %d", pid, pagina);
}

void actualizar_cache_a_memoria(uint32_t pid, int socket_memoria) {
    // Recorrer todas las entradas de la caché
    for (int i = 0; i < cache->capacidad; i++) {
        // Verificar si la entrada está modificada
        if (cache->entradas[i].modificado) {
            // Consultar la dirección física de la página
            uint32_t direccion_fisica = traducir_direccion_logica(cache->entradas[i].pagina, pid, socket_memoria);

            // Enviar el contenido a la memoria
            if (cache->entradas[i].contenido != NULL) {
                //escribir_en_memoria(socket_memoria, direccion_fisica, cache->entradas[i].contenido); // Escribir en memoria
                log_debug(logger_cpu, "## PID: %d - Actualizando en memoria: Página: %d, Dirección: %d", pid, cache->entradas[i].pagina, direccion_fisica);
            }
        }
    }
    log_debug(logger_cpu, "Caché de páginas actualizada en memoria para el proceso PID: %d.", pid);
}

void reemplazar_pagina(uint32_t pagina, uint32_t pid) {
    bool encontrado = false; // Flag para indicar si se encontró una página para reemplazar

    if (strcmp(cache->algoritmo_reemplazo, "CLOCK") == 0) { // Algoritmo CLOCK
        while (!encontrado) {
            // Verificar el bit de uso de la página en la posición del puntero
            if (!cache->entradas[cache->puntero].uso) {
                // Si U=0, se puede reemplazar esta página
                log_debug(logger_cpu, "PID: %d - Reemplazando página: %d", pid, cache->entradas[cache->puntero].pagina);
                
                // Si la página ha sido modificado (M=1), actualizar en memoria
                if (cache->entradas[cache->puntero].modificado) {
                    // Lógica para enviar el contenido a la memoria
                    log_debug(logger_cpu, "PID: %d - Actualizando en memoria página: %d", pid, cache->entradas[cache->puntero].pagina);
                }

                // Reemplazar la página
                cache->entradas[cache->puntero].pagina = pagina;
                cache->entradas[cache->puntero].contenido = NULL; // Aquí deberías asignar el nuevo contenido
                cache->entradas[cache->puntero].modificado = false; // Inicialmente no modificado
                cache->entradas[cache->puntero].uso = true; // Marco como utilizada
                encontrado = true; // Se encontró una página para reemplazar
            } else {
                // Si U=1, limpiarlo y mover al siguiente marco
                cache->entradas[cache->puntero].uso = false;
            }

            // Mueve el puntero al siguiente marco en la caché
            cache->puntero = (cache->puntero + 1) % cache->capacidad;
        }
    } else if (strcmp(cache->algoritmo_reemplazo, "CLOCK-M") == 0) { // Algoritmo CLOCK Modificado
        while (!encontrado) {
            // Verifico el bit de uso y el bit de modificado de la página en la posición del puntero
            if (!cache->entradas[cache->puntero].uso && !cache->entradas[cache->puntero].modificado) {
                // (u=0; m=0): No accedido recientemente, no modificado
                log_debug(logger_cpu, "PID: %d - Reemplazando página: %d", pid, cache->entradas[cache->puntero].pagina);
                
                // Reemplazar la página
                cache->entradas[cache->puntero].pagina = pagina;
                cache->entradas[cache->puntero].contenido = NULL; // HAY QUE ASIGNAR EL NUEVO CONTENIDO TRAIDO DE MEMO
                cache->entradas[cache->puntero].modificado = false;
                cache->entradas[cache->puntero].uso = true;
                encontrado = true;
            } else if (!cache->entradas[cache->puntero].uso && cache->entradas[cache->puntero].modificado) {
                // (u=0; m=1): No accedido recientemente, modificado
                log_debug(logger_cpu, "PID: %d - Actualizando en memoria página: %d", pid, cache->entradas[cache->puntero].pagina);
                // Lógica para enviar el contenido a la memoria
                // ACTUALIZAR MEMORIA

                cache->entradas[cache->puntero].pagina = pagina;
                cache->entradas[cache->puntero].contenido = NULL; // HAY QUE ASIGNAR EL NUEVO CONTENIDO TRAIDO DE MEMO
                cache->entradas[cache->puntero].modificado = false;
                cache->entradas[cache->puntero].uso = true;
                encontrado = true;
            } else if (cache->entradas[cache->puntero].uso && !cache->entradas[cache->puntero].modificado) {
                // (u=1; m=0): Accedido recientemente, no modificado
                cache->entradas[cache->puntero].uso = false; // Limpia el bit de uso
            } else {
                // (u=1; m=1): Accedido recientemente, modificado
                cache->entradas[cache->puntero].uso = false; // Limpiar el bit de uso
                log_debug(logger_cpu, "PID: %d - Actualizando en memoria página: %d", pid, cache->entradas[cache->puntero].pagina);
                // Lógica para enviar el contenido a la memoria
                // ACTUALIZAR MEMORIA
            }

            cache->puntero = (cache->puntero + 1) % cache->capacidad;
        }
    } else {
        log_error(logger_cpu, "PID: %d - Algoritmo de reemplazo no reconocido: %s", pid, cache->algoritmo_reemplazo);
    }
}

void escribir_en_cache(uint32_t direccion_logica, const char* datos, uint32_t pid, int socket_memoria) {
    uint32_t pagina = obtener_numero_pagina(direccion_logica); // Función para obtener el número de página (AGREGAR A traduccion.c)
    if (acceder_pagina_cache(pagina, pid)) { // Cache Hit
        // Si la página está en la caché, escribe los datos
        cache->entradas[cache->puntero].contenido = strdup(datos); // Asigno los datos a la entrada de la caché
        cache->entradas[cache->puntero].modificado = true; // Marco como modificada
        log_debug(logger_cpu, "## PID: %d - Escribiendo en caché: Página: %d, Datos: %s", pid, pagina, datos);
    } else { // Cache Miss
        // Si no está en la caché, cargo la página desde la memoria
        cargar_pagina_en_cache(pagina, pid, socket_memoria);
        // Escribo los datos
        cache->entradas[cache->puntero].contenido = strdup(datos);
        cache->entradas[cache->puntero].modificado = true;
        log_debug(logger_cpu, "## PID: %d - Página no encontrada en caché, cargando desde memoria: Página: %d, Datos: %s", pid, pagina, datos);
    }
}

char* leer_de_cache(uint32_t direccion_logica, uint32_t tamanio, uint32_t pid, int socket_memoria) {
    uint32_t pagina = obtener_numero_pagina(direccion_logica); // Función para obtener el número de página
    const char* datos; 
    if (acceder_pagina_cache(pagina, pid)) { // Cache Hit
        // Si la página está en la caché, leer los datos
        datos = cache->entradas[cache->puntero].contenido; // Obtener los datos de la entrada de la caché
        log_debug(logger_cpu, "## PID: %d - Leyendo de caché: Página: %d, Datos: %s", pid, pagina, datos);
    } else { // Cache Miss
        // Si no está en la caché, cargar la página desde la memoria
        cargar_pagina_en_cache(pagina, pid, socket_memoria);
        // Leer los datos
        datos = cache->entradas[cache->puntero].contenido; // Obtener los datos de la entrada de la caché
        log_debug(logger_cpu, "## PID: %d - Página no encontrada en caché, cargando desde memoria: Página: %d, Datos: %s", pid, pagina, datos);
    }
    printf("PID: %d - Acción: LEER - Página: %d - Valor: %s", pid, pagina, datos);
    return datos;
}

void limpiar_cache() {
    if (!acceder_cache()) { // Verifico si la caché está habilitada
        log_debug(logger_cpu, "Caché deshabilitada o no inicializada, no se puede limpiar.");
        return; // No hay nada que limpiar
    }
    // Limpio todas las entradas de la caché
    for (int i = 0; i < cache->capacidad; i++) {
        // Liberar el contenido de la entrada si es necesario
        if (cache->entradas[i].contenido != NULL) {
            free(cache->entradas[i].contenido); // Liberar memoria asignada
            cache->entradas[i].contenido = NULL; // Restablecer el puntero
        }
        // Restablezco otros campos
        cache->entradas[i].pagina = 0;
        cache->entradas[i].modificado = false;
        cache->entradas[i].uso = false;
        cache->puntero = 0;
    }
    log_debug(logger_cpu, "Caché de páginas limpia.");
}
