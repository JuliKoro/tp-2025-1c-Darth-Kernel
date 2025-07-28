#include "traduccion.h"

t_tabla_pag* info_tabla_pag;
tlb_t* tlb;
uint32_t contador_accesos = 0;

uint32_t traducir_direccion_logica(uint32_t direccion_logica, uint32_t pid, int socket_memoria) {
    // Calcular el número de página y el desplazamiento
    uint32_t nro_pagina = floor(direccion_logica / info_tabla_pag->tamanio_pagina);
    uint32_t desplazamiento = direccion_logica % info_tabla_pag->tamanio_pagina;

    // Calcular las entradas de cada nivel
    uint32_t entradas_niveles[info_tabla_pag->cant_niveles];
    for (uint32_t i = 0; i < info_tabla_pag->cant_niveles; i++) {
        entradas_niveles[i] = (uint32_t)floor(nro_pagina / pow(info_tabla_pag->cant_entradas_tabla, info_tabla_pag->cant_niveles - 1 - i)) % info_tabla_pag->cant_entradas_tabla;
    }

    // Consultar TLB
    int32_t posicion = consultar_tlb(pid, nro_pagina);
    if (posicion != -1) {
        uint32_t marco_tlb = tlb->entradas[posicion].marco;
        // TLB Hit
        log_info(logger_cpu, "PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);

        return (marco_tlb * info_tabla_pag->tamanio_pagina) + desplazamiento; // Devolver dirección física
    }
    else{
        // TLB Miss
        log_info(logger_cpu, "PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);

        // Consultar memoria para obtener el marco
        int32_t marco_memoria = obtener_marco_de_memoria(nro_pagina, socket_memoria, pid);
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

int32_t consultar_tlb(uint32_t pid, uint32_t numero_pagina) { //Devuelve la posicion en el array de la tlb o -1
    contador_accesos ++;
    uint32_t posicion = 0;
    while(posicion < tlb->tamaño && (tlb->entradas[posicion].pid != pid || tlb->entradas[posicion].pagina != numero_pagina)){
        posicion ++;
    }
    if(posicion == tlb->tamaño){ //Si no lo encuentra devuelve -1
        return -1;
    }
    else{
        tlb->entradas[posicion].ultimo_uso = contador_accesos; //Actualizo su último uso
        return posicion; 
    }
}

void agregar_a_tlb(uint32_t numero_pagina, uint32_t marco_tlb, uint32_t pid) {

    // Si hay lugar lo agrega al final
    if(tlb->capacidad > tlb->tamaño){
        tlb->entradas[tlb->tamaño].marco = marco_tlb;
        tlb->entradas[tlb->tamaño].pagina = numero_pagina;
        tlb->entradas[tlb->tamaño].pid = pid;
        tlb->entradas[tlb->tamaño].ingreso = contador_accesos;
        tlb->entradas[tlb->tamaño].ultimo_uso = contador_accesos;
        tlb->tamaño ++;
    }
    // Sino se reemplaza
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
        while(posicion_aux < tlb->tamaño){
            if(tlb->entradas[posicion_aux].ingreso < tlb->entradas[posicion_reemplazo].ingreso){
                posicion_reemplazo = posicion_aux;
            }
            posicion_aux ++;
        }
    }

    //En caso que sea LRU
    else{
        while(posicion_aux < tlb->tamaño){
            if(tlb->entradas[posicion_aux].ultimo_uso < tlb->entradas[posicion_reemplazo].ultimo_uso){
                posicion_reemplazo = posicion_aux;
            }
            posicion_aux ++;
        }    
    }

    //Reemplazo la página encontrada
    tlb->entradas[posicion_reemplazo].marco = marco_tlb;
    tlb->entradas[posicion_reemplazo].pagina = numero_pagina;
    tlb->entradas[posicion_reemplazo].pid = pid;
    tlb->entradas[posicion_reemplazo].ingreso = contador_accesos;
    tlb->entradas[posicion_reemplazo].ultimo_uso = contador_accesos;
}

uint32_t obtener_marco_de_memoria(uint32_t numero_pagina, int socket_memoria, uint32_t pid) {
    // Lógica para enviar un paquete a memoria y recibir el marco correspondiente
    uint32_t marco_memoria;
    //uint32_t marco_memoria = ...; // Lógica para obtener el marco de memoria
    return marco_memoria;
}

uint32_t obtener_numero_pagina(uint32_t direccion_logica) {
    return floor(direccion_logica / info_tabla_pag->tamanio_pagina);
}