#include "planificacion-corto-plazo.h"



void iniciar_planificador_corto_plazo() {
    //Obtengo el enum correspondiente al algoritmo de planificacion
    algoritmos_de_planificacion algoritmo = obtener_algoritmo_de_planificacion(kernel_configs.cortoplazo);
    
    switch (algoritmo) {
        case FIFO:
            while(true) {
            //1. Envio proceso desde cola NEW a cola READY
            t_pcb* pcb = obtener_pcb_de_lista_new();
            if(pcb == NULL) {
                log_error(logger_kernel, "Error al obtener pcb de lista new");
                continue;
                }
                agregar_pcb_a_lista_ready(pcb);

                //2. Envio proceso desde cola READY a cola EXECUTING
                //TODO: erifico si hay CPUs disponibles
                pcb = peek_lista_ready();
                if(pcb == NULL) {
                    log_error(logger_kernel, "Error al obtener pcb de lista ready");
                    continue;
                }
                //agregar_pcb_a_cola_executing(pcb);
                
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
