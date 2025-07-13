#include "planificacion-largo-plazo.h"


void* iniciar_planificador_largo_plazo() {
    
    //Obtengo el enum correspondiente al algoritmo de planificacion
    algoritmos_de_planificacion algoritmo = obtener_algoritmo_de_planificacion(kernel_configs.ingreasoaready);
    
    switch (algoritmo) {
        case FIFO:
            while(true) {
                
                sem_wait(&sem_procesos_en_new); //Si no hay procesos en new, se bloquea el hilo

                //1. Verifico si hay procesos en cola running que hayan terminado, si terminaron se mueven a la cola exit
                //Puede no haberlos
                mover_procesos_terminados(); 

                //2. Verifico si hay procesos en cola new
                //TODO: Verificar que no haya procesos en la cola de susp ready
                t_pcb* pcb = peek_lista_new();
                if(pcb == NULL) {
                    log_error(logger_kernel, "Error al peekear la lista new");
                    continue;
                }
                if(solicitar_creacion_proceso(pcb)) { 
                    pcb = obtener_pcb_de_lista_new();
                    mover_a_ready(pcb);
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

   return NULL;
}