#include "pcb.h"

u_int32_t pid_counter = 0;

t_pcb* inicializar_pcb(u_int32_t pid, char* archivo_pseudocodigo, u_int32_t tamanio_proceso) {
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->pid = pid;
    pcb->archivo_pseudocodigo = archivo_pseudocodigo;
    pcb->tamanio_proceso = tamanio_proceso;
    pcb->metricas_estado = list_create();
    pcb->metricas_tiempo = list_create();
    pcb->estado = NEW;
    pcb->pc = 0;
    return pcb;
}