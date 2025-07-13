#include "pcb.h"

u_int32_t pid_counter = 0;

t_pcb* inicializar_pcb(u_int32_t pid, char* archivo_pseudocodigo, u_int32_t tamanio_proceso) {
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->pid = pid;
    pcb->archivo_pseudocodigo = strdup(archivo_pseudocodigo);
    pcb->tamanio_proceso = tamanio_proceso;
    pcb->estado = NEW;
    pcb->pc = 0;
    gettimeofday(&pcb->ult_update, NULL);


    for (int i = 0; i < ESTADOS_COUNT; i++) {
        pcb->metricas_estado[i].estado = i;
        pcb->metricas_estado[i].contador = 0;
    }

    for (int i = 0; i < ESTADOS_COUNT; i++) {
        pcb->metricas_tiempo[i].estado = i;
        pcb->metricas_tiempo[i].tiempo_acumulado = 0;
    }

    pcb->metricas_estado[NEW].contador = 1;

    pthread_mutex_init(&pcb->mutex_cambio_estado, NULL);

    return pcb;
}



