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

// Traduccion JULI
t_tabla_pag* info_tabla_pag;
tlb_t* tlb;

uint32_t traducir_direccion_logica(uint32_t direccion_logica, uint32_t pid, int socket_memoria) {
    // Calcular el número de página y el desplazamiento
    uint32_t nro_pagina = floor(direccion_logica / info_tabla_pag->tamanio_pagina);
    uint32_t desplazamiento = direccion_logica % info_tabla_pag->tamanio_pagina;

    // Calcular las entradas de cada nivel
    uint32_t entradas_niveles[info_tabla_pag->cant_niveles];
    for (uint32_t i = 0; i < info_tabla_pag->cant_niveles; i++) {
        entradas_niveles[i] = floor(nro_pagina / pow(info_tabla_pag->cant_entradas_tabla, info_tabla_pag->cant_niveles - 1 - i)) % info_tabla_pag->cant_entradas_tabla;
    }

    // Consultar TLB
    uint32_t marco_tlb;
    if (consultar_tlb(nro_pagina, &marco_tlb)) {
        // TLB Hit
        log_info(logger_cpu, "PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);
        return (marco_tlb * info_tabla_pag->tamanio_pagina) + desplazamiento; // Devolver dirección física
    }
    // TLB Miss
    log_info(logger_cpu, "PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);

    // Consultar memoria para obtener el marco
    uint32_t marco_memoria = obtener_marco_de_memoria(nro_pagina, socket_memoria, pid);
    if (marco_memoria == -1) {
        // Manejo de error: no se pudo obtener el marco
        return -1; // O manejar el error de otra manera
    }

    // Registrar el log de obtención de marco
    log_info(logger_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, nro_pagina, marco_memoria);

    // Agregar a la TLB
    agregar_a_tlb(nro_pagina, marco_memoria, pid);

    return (marco_memoria * info_tabla_pag->tamanio_pagina) + desplazamiento; // Devolver dirección física
}

tlb_t* crear_tlb(uint32_t capacidad) {
    tlb_t* tlb = malloc(sizeof(tlb_t));
    tlb->entradas = malloc(sizeof(cpu_configs.entradastlb) * capacidad);
    tlb->capacidad = capacidad;
    tlb->tamaño = 0;
    tlb->algoritmo_reemplazo = cpu_configs.reemplazotlb; // FIFO o LRU
    return tlb;
}

void destruir_tlb() {

}

bool consultar_tlb(uint32_t numero_pagina, uint32_t* marco_tlb) {

}

void agregar_a_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid) {

}

void reemplazar_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid) {

}

uint32_t obtener_marco_de_memoria(uint32_t numero_pagina, int socket_memoria, uint32_t pid) {
    // Lógica para enviar un paquete a memoria y recibir el marco correspondiente
    uint32_t marco_memoria;
    //uint32_t marco_memoria = ...; // Lógica para obtener el marco de memoria
    return marco_memoria;
}

