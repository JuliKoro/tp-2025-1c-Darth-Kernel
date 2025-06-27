#include "planificacion-largo-plazo.h"

algoritmo_largo_plazo obtener_algoritmo_largo_plazo(char* algoritmo) {
   if (strcmp(algoritmo, "FIFO") == 0) {
      return FIFO;
   }
   return PMCP;
}
void* iniciar_planificador_largo_plazo() {
    
    
    algoritmo_largo_plazo algoritmo = obtener_algoritmo_largo_plazo(kernel_configs.ingreasoaready);
    
    switch (algoritmo) {
        case FIFO:
            
            while(true) {
                log_info(logger_kernel, "Planificador largo plazo iniciado en FIFO");
                sem_wait(&sem_procesos_en_new); //Si no hay procesos en new, se bloquea el hilo

                //1. Verifico si hay procesos en cola running que hayan terminado, si terminaron se mueven a la cola exit
                //Puede no haberlos
                mover_procesos_terminados(); 

                //2. Verifico si hay procesos en cola new
                t_pcb* pcb = peek_cola_new();
                if(pcb == NULL) {
                    log_error(logger_kernel, "Error al peekear la cola new");
                    continue;
                }
                if(solicitar_creacion_proceso(pcb)) { 
                    pcb = obtener_pcb_de_cola_new();
                    agregar_pcb_a_cola_ready(pcb);
                    log_info(logger_kernel, "## (%d) Pasa del estado NEW al estado READY", pcb->pid);
                }
            }
        break;
        case PMCP:
            printf("PMCP No implementado aun\n");
            break;
        default:
            printf("No tenido en cuenta o incorrecto\n");
        break;
   }
}