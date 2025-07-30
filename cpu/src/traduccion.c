#include "traduccion.h"

tlb_t* tlb;
uint32_t contador_accesos = 0;

uint32_t traducir_direccion_logica(uint32_t direccion_logica, uint32_t pid, int socket_memoria) {
    // Calcula el #pág y el desplazamiento (offset)
    uint32_t nro_pagina = obtener_numero_pagina(direccion_logica);
    uint32_t desplazamiento = direccion_logica % cpu_configs.tamanio_pagina;

    // Consultar TLB
    if(acceder_tlb()) {
        int32_t posicion = consultar_tlb(pid, nro_pagina);
        if (posicion != -1) {
            uint32_t marco_tlb = tlb->entradas[posicion].marco;
            // TLB Hit
            log_info(logger_cpu, "PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);
            return (marco_tlb * cpu_configs.tamanio_pagina) + desplazamiento; // Devuelve DF
        }
        else{
            // TLB Miss
            log_info(logger_cpu, "PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);
        }
    }
    
    // Calcular las entradas de cada nivel
    uint32_t entradas_niveles[cpu_configs.cant_niveles];
    for (uint32_t i = 0; i < cpu_configs.cant_niveles; i++) {
        entradas_niveles[i] = (uint32_t)floor(nro_pagina / pow(cpu_configs.cant_entradas_tabla, cpu_configs.cant_niveles - 1 - i)) % cpu_configs.cant_entradas_tabla;
    }

    // Consultar memoria para obtener el marco
    int32_t marco_memoria = obtener_marco_de_memoria(nro_pagina, entradas_niveles, pid, socket_memoria);
    if (marco_memoria == -1) {
        log_error(logger_cpu, "Error al obtener marco de memoria para PID: %d, Pagina: %d", pid, nro_pagina);
        return -1; // Manejo de error
    }

    // Agregar a la TLB
    agregar_a_tlb(nro_pagina, marco_memoria, pid);

    return (marco_memoria * cpu_configs.tamanio_pagina) + desplazamiento; // Devuelve DF
}

uint32_t obtener_marco_de_memoria(uint32_t numero_pagina, uint32_t* entradas_niveles, uint32_t pid, int socket_memoria) {
    uint32_t marco_obtenido = -1; // Valor por defecto para indicar error

    // Construyo el paquete para Memoria
    t_entradas_tabla* entrada_tabla;
    entrada_tabla->pid = pid;
    entrada_tabla->entradas_niveles = entradas_niveles;
    entrada_tabla->num_pag = numero_pagina;

    t_buffer* buffer = serializar_solicitud_marco(entrada_tabla, cpu_configs.cant_niveles);

    t_paquete* paquete_solicitud;
    paquete_solicitud->buffer = buffer;
    paquete_solicitud->codigo_operacion = PAQUETE_SOLICITUD_MARCO;

    // Enviar el paquete a Memoria
    if (enviar_paquete(socket_memoria, paquete_solicitud) == -1) {
        log_error(logger_cpu, "Error al enviar solicitud de marco a Memoria para PID: %d, Pagina: %d", pid, numero_pagina);
        return -1;
    }

    // Recibo la respuesta de Memoria
    if (recibir_marco(socket_memoria, &marco_obtenido) == -1) {
        log_error(logger_cpu, "Error al recibir el marco de Memoria para PID: %d, Pagina: %d", pid, numero_pagina);
        return -1; // Manejo de error
    }

    if (marco_obtenido == -1) {
        log_error(logger_cpu, "Memoria reportó un error al obtener marco para PID: %d, Pagina: %d", pid, numero_pagina);
        return -1;
    }

    log_info(logger_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, numero_pagina, marco_obtenido);
    return marco_obtenido;
}

tlb_t* crear_tlb(uint32_t capacidad) {
    tlb_t* tlb = malloc(sizeof(tlb_t));
    tlb->entradas = malloc(sizeof(t_entrada_tlb) * capacidad);
    tlb->capacidad = capacidad;
    tlb->tamanio = 0;
    tlb->algoritmo_reemplazo = cpu_configs.reemplazotlb; // FIFO o LRU
    return tlb;
}

void destruir_tlb() {
    if (tlb != NULL) {
        free(tlb->entradas); // Libera las entradas de la TLB
        free(tlb); // Libera la estructura de la TLB
        tlb = NULL; // Evita un puntero colgante
    }
}

void limpiar_tlb(uint32_t pid) {
    for (uint32_t i = 0; i < tlb->tamanio; i++) {
        if (tlb->entradas[i].pid == pid) {
            // Si encuentra una entrada que corresponde al PID, se elimina
            tlb->entradas[i] = tlb->entradas[tlb->tamanio - 1]; // Se mueve la última entrada a la posición actual
            tlb->tamanio--; // Reduzco el tamaño de la TLB
            i--; // Volver a verificar la posición actual
        }
    }
}

bool acceder_tlb() {
    // Verificar si la TLB está habilitada
    if (tlb == NULL || tlb->tamanio <= 0) {
        log_debug(logger_cpu, "TLB deshabilitada, accediendo directamente a Tabla de Paginas de Memoria.");
        return false; // La TLB está deshabilitada, no se puede acceder
    }
    return true; // TLB habilitada
}

int32_t consultar_tlb(uint32_t pid, uint32_t numero_pagina) { //Devuelve la posicion en el array de la tlb o -1
    contador_accesos ++;
    uint32_t posicion = 0;
    while(posicion < tlb->tamanio && (tlb->entradas[posicion].pid != pid || tlb->entradas[posicion].pagina != numero_pagina)){
        posicion ++;
    }
    if(posicion == tlb->tamanio){ //Si no lo encuentra devuelve -1
        return -1;
    }
    else{
        tlb->entradas[posicion].ultimo_uso = contador_accesos; //Actualizo su último uso
        return posicion; 
    }
}

void agregar_a_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid) {

    // Si hay lugar lo agrega al final
    if(tlb->capacidad > tlb->tamanio){
        tlb->entradas[tlb->tamanio].marco = marco_tlb;
        tlb->entradas[tlb->tamanio].pagina = numero_pagina;
        tlb->entradas[tlb->tamanio].pid = pid;
        tlb->entradas[tlb->tamanio].ingreso = contador_accesos;
        tlb->entradas[tlb->tamanio].ultimo_uso = contador_accesos;
        tlb->tamanio ++;
    }
    // Si no se reemplaza
    else{
        reemplazar_tlb(numero_pagina, marco_tlb, pid);
    }

}

void reemplazar_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid){
    uint32_t posicion_aux = 0;
    uint32_t posicion_reemplazo = 0; //Posición de la página a reemplazar

    //Busco la posición de la página que voy a reemplazar

    //En caso que sea FIFO
    if(strcmp(tlb->algoritmo_reemplazo, "FIFO") == 0){
        while(posicion_aux < tlb->tamanio){
            if(tlb->entradas[posicion_aux].ingreso < tlb->entradas[posicion_reemplazo].ingreso){
                posicion_reemplazo = posicion_aux;
            }
            posicion_aux ++;
        }
    }

    //En caso que sea LRU
    else if(strcmp(tlb->algoritmo_reemplazo, "LRU") == 0){
        while(posicion_aux < tlb->tamanio){
            if(tlb->entradas[posicion_aux].ultimo_uso < tlb->entradas[posicion_reemplazo].ultimo_uso){
                posicion_reemplazo = posicion_aux;
            }
            posicion_aux ++;
        }    
    } else {
        log_error(logger_cpu, "PID: %d - Algoritmo de reemplazo no reconocido: %s", pid, tlb->algoritmo_reemplazo);
    }

    //Reemplazo la página encontrada
    tlb->entradas[posicion_reemplazo].marco = marco_tlb;
    tlb->entradas[posicion_reemplazo].pagina = numero_pagina;
    tlb->entradas[posicion_reemplazo].pid = pid;
    tlb->entradas[posicion_reemplazo].ingreso = contador_accesos;
    tlb->entradas[posicion_reemplazo].ultimo_uso = contador_accesos;
}

uint32_t obtener_numero_pagina(uint32_t direccion_logica) {
    return floor(direccion_logica / cpu_configs.tamanio_pagina);
}