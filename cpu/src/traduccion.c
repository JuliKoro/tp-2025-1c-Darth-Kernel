#include "traduccion.h"

/* Traduccion NICO
t_direccion_fisica direccion_logica_a_fisica(uint32_t direccion_logica){
    t_direccion_fisica direccion_fisica;
    //Obtengo los valores de la dirección física
    direccion_fisica.nro_pagina = floor(direccion_logica / cpu_configs.tampagina); //El tamaño de página lo agregué a las configs
    //direccion_fisica.entrada_nivel_X = floor(direccion_fisica.nro_pagina  / cpu_configs.entradastlb ^ (N - X)) % cpu_configs.entradastlb
    direccion_fisica.desplazamiento = direccion_logica % cpu_configs.tampagina;
    return direccion_fisica;
}
*/
/* Traduccion JULI
uint32_t traducir_direccion_logica(uint32_t direccion_logica, uint32_t pid, int socket_memoria, t_tabla_pag* info_tabla_pag) {
    // Calcular el número de página y el desplazamiento
    uint32_t nro_pagina = floor(direccion_logica / info_tabla_pag->tamanio_pagina);
    uint32_t desplazamiento = direccion_logica % info_tabla_pag->tamanio_pagina;

    // Consultar TLB
    uint32_t marco_tlb;
    if (consultar_tlb(nro_pagina, &marco_tlb)) {
        // TLB Hit
        log_info(logger_cpu, "PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);
        return (marco_tlb * info_tabla_pag->tamanio_pagina) + desplazamiento; // DF
    }

    // TLB Miss
    log_info(logger_cpu, "PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);

    // Consultar caché
    uint32_t marco_cache;
    if (consultar_cache(nro_pagina, &marco_cache)) {
        // Cache Hit
        log_info(logger_cpu, "PID: %d - Cache Hit - Pagina: %d", pid, nro_pagina);
        return (marco_cache * info_tabla_pag->tamanio_pagina) + desplazamiento; // DF
    }

    // Cache Miss
    log_info(logger_cpu, "PID: %d - Cache Miss - Pagina: %d", pid, nro_pagina);

    // Consultar memoria para obtener el marco
    uint32_t marco_memoria = obtener_marco_de_memoria(nro_pagina, socket_memoria);
    if (marco_memoria == -1) {
        // Manejo de error: no se pudo obtener el marco
        return -1; // O manejar el error de otra manera
    }

    // Agregar a la TLB y a la caché
    agregar_a_tlb(nro_pagina, marco_memoria, pid);
    agregar_a_cache(nro_pagina, marco_memoria);

    return (marco_memoria * info_tabla_pag->tamanio_pagina) + desplazamiento;
}

uint32_t obtener_marco_de_memoria(uint32_t numero_pagina, int socket_memoria) {
    // Lógica para enviar un paquete a memoria y recibir el marco correspondiente
    // (Se asume que se recibe un marco válido)
    // uint32_t marco_memoria = ...; // Lógica para obtener el marco de memoria
    // log_info(logger_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, numero_pagina, marco_memoria);
    // return marco_memoria;
}

bool consultar_cache(uint32_t numero_pagina, uint32_t* marco_cache) {
    // Lógica para consultar la caché de páginas
    // Si se encuentra, asignar el marco correspondiente y devolver true
    return false; // Cambiar según la lógica de consulta
}

void agregar_a_cache(uint32_t numero_pagina, uint32_t marco_cache) {
    // Lógica para agregar una entrada a la caché
    // log_info(logger_cpu, "PID: %d - Cache Add - Pagina: %d", pid, numero_pagina);
}

void actualizar_cache_a_memoria(uint32_t numero_pagina, uint32_t marco_cache, int socket_memoria) {
    // Lógica para actualizar la caché a memoria
    // log_info(logger_cpu, "PID: %d - Memory Update - Página: %d - Frame: %d", pid, numero_pagina, marco_cache);
}
*/