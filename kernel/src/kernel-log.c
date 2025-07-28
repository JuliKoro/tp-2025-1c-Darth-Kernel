#include "kernel-log.h"

t_log* logger_kernel;

void inicializar_logger_kernel() {
    logger_kernel = log_create("kernel.log", "Kernel", true, log_level_from_string(kernel_configs.loglevel));
}

void destruir_logger_kernel() {
    //Chequeo que el logger exista y lo destruyo
    if (logger_kernel != NULL) {
        log_destroy(logger_kernel);
    }
}

void log_fin_io(t_pcb* pcb) {
    log_info(logger_kernel, "## (<%d>) finalizó IO y pasa a READY", pcb->pid);
    //Este log esta en el manejador de IO, mas facil de implementar ahi

}

void log_cambio_estado(t_pcb* pcb, estado_pcb estado_anterior, estado_pcb estado_nuevo) {
    log_info(logger_kernel, "## (<%d>) Pasa del estado %s al estado <%s>", pcb->pid, estado_pcb_to_string(estado_anterior), estado_pcb_to_string(estado_nuevo));
}

void log_fin_proceso(t_pcb* pcb) {
    log_info(logger_kernel, "## (<%d>) - Finaliza el proceso", pcb->pid);
}

void log_solicitud_syscall(u_int32_t pid, char* syscall) {
    log_info(logger_kernel, "## (<%d>) - Solicitó syscall: <%s>", pid, syscall);
}

void log_creacion_proceso(t_pcb* pcb) {
    log_info(logger_kernel, "## (<%d>) - Se crea el proceso - Estado: NEW", pcb->pid);
}

void log_metricas_estado(t_pcb* pcb) {
    log_info(logger_kernel, "## (<%d>) - Metricas de estado: NEW (%d) (%f), READY (%d) (%f), RUNNING (%d) (%f), BLOCKED (%d) (%f), SUSP_BLOCKED (%d) (%f), SUSP_READY (%d) (%f), EXIT (%d) (%f)", 
    pcb->pid, 
    pcb->metricas_estado[NEW].contador,
    pcb->metricas_tiempo[NEW].tiempo_acumulado,

    pcb->metricas_estado[READY].contador,
    pcb->metricas_tiempo[READY].tiempo_acumulado,

    pcb->metricas_estado[RUNNING].contador,
    pcb->metricas_tiempo[RUNNING].tiempo_acumulado,

    pcb->metricas_estado[BLOCKED].contador,
    pcb->metricas_tiempo[BLOCKED].tiempo_acumulado,

    pcb->metricas_estado[SUSP_BLOCKED].contador,
    pcb->metricas_tiempo[SUSP_BLOCKED].tiempo_acumulado,

    pcb->metricas_estado[SUSP_READY].contador,
    pcb->metricas_tiempo[SUSP_READY].tiempo_acumulado,

    pcb->metricas_estado[EXIT].contador,
    pcb->metricas_tiempo[EXIT].tiempo_acumulado);


}

void log_desalojo(t_pcb* pcb){
    log_info(logger_kernel, "## (<%d>) - Desalojado por el algoritmo SJF", pcb->pid);
}

void log_motivo_bloqueo(u_int32_t pid, char* nombreio) {
    log_info(logger_kernel, "## (<%d>) - Bloqueado por IO: <%s>", pid, nombreio);
}

const char* estado_pcb_to_string(estado_pcb estado) {
    switch (estado) {
        case NEW: return "NEW";
        case READY: return "READY";
        case RUNNING: return "RUNNING";
        case BLOCKED: return "BLOCKED";
        case SUSP_BLOCKED: return "SUSP_BLOCKED";
        case SUSP_READY: return "SUSP_READY";
        case EXIT: return "EXIT";
        default: return "DESCONOCIDO";
    }
}