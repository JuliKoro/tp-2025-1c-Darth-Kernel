#ifndef PCB_H
#define PCB_H

#include <commons/collections/list.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>


extern u_int32_t pid_counter;

typedef enum {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    EXIT
} estado_pcb;

typedef struct t_pcb {
    u_int32_t pid;
    u_int32_t pc;
    t_list* metricas_estado;
    t_list* metricas_tiempo;
    char* archivo_pseudocodigo;
    u_int32_t tamanio_proceso;
    estado_pcb estado;
} t_pcb;

estado_pcb obtener_estado_pcb(u_int32_t estado);

t_pcb* inicializar_pcb(u_int32_t pid, char* archivo_pseudocodigo, u_int32_t tamanio_proceso);

#endif
