#include "planificacion-corto-plazo.h"



void iniciar_planificador_corto_plazo() {
    //Obtengo el enum correspondiente al algoritmo de planificacion
    algoritmos_de_planificacion algoritmo = obtener_algoritmo_de_planificacion(kernel_configs.cortoplazo);
    
    switch (algoritmo) {
        case FIFO:
            while(true) {
                //Obtengo el pcb de lista ready, ya esta ordenado segun el algoritmo
                sem_wait(&sem_corto_plazo); //Espero a que haya un pcb en ready
                sem_wait(&sem_cpu_disponible); //Espero a que haya una cpu disponible
                t_pcb* pcb = peek_lista_ready();
                if(pcb == NULL) {
                    log_error(logger_kernel, "No hay nada en ready aun");
                    continue;
                }
                //Esta funcion asigna el pcb a un cpu y lo manda a la cola executing
                asignar_pcb_a_cpu(pcb);
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
