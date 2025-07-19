#include "planificacion-largo-plazo.h"


void* iniciar_planificador_largo_plazo() {
        while(true) {

        sem_wait(&sem_largo_plazo); //Si no hay procesos en new, se bloquea el hilo

            while(true) {
                //1. Verifico si hay procesos en cola running que hayan terminado, si terminaron se mueven a la cola exit
                //Puede no haberlos
                mover_procesos_terminados();
                eliminar_procesos_en_exit();

                //2. Me fijo si hay procesos en suspready
                t_pcb* pcb = obtener_pcb_de_lista_suspready();
                if(pcb != NULL /* &&  sacar de swap*/) {
                    mover_a_ready(pcb);
                    continue;
                }
                //3. Me fijo si hay procesos en new
                t_pcb* pcb_new = peek_lista_new();
                if(pcb_new == NULL) {
                    break;
                }
                if (solicitar_creacion_proceso(pcb_new)) {
                    pcb_new = obtener_pcb_de_lista_new();
                    mover_a_ready(pcb_new);
                    sem_post(&sem_largo_plazo);
                    continue;
                } else {
                    log_warning(logger_kernel, "Memoria llena. PID %d no puede ser admitido.", pcb_new->pid);
                    log_info(logger_kernel, "Planificador en espera de liberacion de memoria");
                    
                    //Espero a que memoria libere memoria
                    sem_wait(&sem_memoria_disponible);
                    continue;
                }
        }
    return NULL;
    }
}
