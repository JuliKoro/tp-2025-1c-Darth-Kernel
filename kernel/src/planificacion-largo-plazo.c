#include "planificacion-largo-plazo.h"

algoritmo_largo_plazo obtener_algoritmo(char* algoritmo) {
   if (strcmp(algoritmo, "FIFO") == 0) {
      return FIFO;
   }
   return PMCP;
}
void iniciar_planificador_largo_plazo() {
    switch (obtener_algoritmo(kernel_configs.ingreasoaready)) {
        case FIFO:
            while(true) {
                //1. Verifico si hay procesos en cola running que hayan terminado
                mover_procesos_terminados();

                //2. Verifico si hay procesos en cola new
                t_pcb* pcb = peek_cola_new();
                if(pcb == NULL) {
                    log_error(logger_kernel, "Error al peekear la cola new");
                    continue;
                }
                if(solicitar_creacion_proceso(pcb)) { 
                    log_info(logger_kernel, "## (%d) Pasa del estado NEW al estado READY");
                    pcb = obtener_pcb_de_cola_new();
                    agregar_pcb_a_cola_ready(pcb);
                }
            }
        break;
        case PMCP:
            printf("No implementado aun\n");
            break;
        default:
            printf("No tenido en cuenta o incorrecto\n");
        break;
   }
}