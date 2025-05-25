#include "planificacion-corto-plazo.h"

void iniciar_planificador_corto_plazo() {
    switch (obtener_algoritmo_corto_plazo(kernel_configs.cortoplazo)) {
        case FIFOX:
            while(true) {
            //1. Envio proceso desde cola NEW a cola READY
            t_pcb* pcb = obtener_pcb_de_cola_new();
            if(pcb == NULL) {
                log_error(logger_kernel, "Error al obtener pcb de cola new");
                continue;
                }
                agregar_pcb_a_cola_ready(pcb);

                //2. Envio proceso desde cola READY a cola EXECUTING
                //TODO: erifico si hay CPUs disponibles
                pcb = obtener_pcb_de_cola_ready();
                if(pcb == NULL) {
                    log_error(logger_kernel, "Error al obtener pcb de cola ready");
                    continue;
                }
                agregar_pcb_a_cola_executing(pcb);
                
                //TODO: Manejar cola de executing, mandar procesos a CPU
                //3. Envio proceso desde cola EXECUTING a cola BLOCKED
                //TODO: Verifico si hay IO disponibles
            }
            break;
        case SJF_SIN_DESALOJO:
            printf("No implementado aun\n");
            break;
        case SFJ_CON_DESALOJO:
            printf("No implementado aun\n");
            break;
        default:
            printf("No tenido en cuenta o incorrecto\n");
            break;
    }
}

algoritmo_corto_plazo obtener_algoritmo_corto_plazo(char* algoritmo) {
    if (strcmp(algoritmo, "FIFO") == 0) {
        return FIFOX;
    }
    if (strcmp(algoritmo, "SJF_SIN_DESALOJO") == 0) {
        return SJF_SIN_DESALOJO;
    }
    return SFJ_CON_DESALOJO;
}