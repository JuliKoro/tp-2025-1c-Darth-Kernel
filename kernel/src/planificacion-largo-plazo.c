#include "planificacion-largo-plazo.h"
#include "kernel-log.h"


void* iniciar_planificador_largo_plazo() {
    log_info(logger_kernel, "PLP-DEBUG: Hilo Planificador Largo Plazo iniciado.");
        while(true) {

        log_info(logger_kernel, "PLP-DEBUG: [Bloqueo] Esperando señal para iniciar ciclo de planificación (sem_largo_plazo).");
        sem_wait(&sem_largo_plazo);
        log_debug(logger_kernel, "[PLP] Señal de sem_largo_plazo recibida."); // DEBUG_LOG
        log_info(logger_kernel, "PLP-DEBUG: [Señal Recibida] Iniciando nuevo ciclo de planificación.");

            while(true) {
                log_info(logger_kernel, "PLP-DEBUG: --- Inicio de ciclo ---");
                //1. Verifico si hay procesos en cola running que hayan terminado, si terminaron se mueven a la cola exit
                //Puede no haberlos
                mover_procesos_terminados();
                eliminar_procesos_en_exit();

                //2. Me fijo si hay procesos en suspready
                log_info(logger_kernel, "PLP-DEBUG: [Paso 1] Verificando cola SUSP_READY (prioritaria)...");
                t_pcb* pcb = obtener_pcb_de_lista_suspready();

                if(pcb != NULL) {
                    log_info(logger_kernel, "PLP-DEBUG: [Acción] Proceso PID %d encontrado en SUSP_READY. Moviendo a READY.", pcb->pid);
                    mover_a_ready(pcb);
                    continue; // Vuelvo al inicio del ciclo para re-evaluar prioridades
                }
                
                log_info(logger_kernel, "PLP-DEBUG: [Paso 1] Cola SUSP_READY está vacía.");

                //3. Me fijo si hay procesos en new
                log_info(logger_kernel, "PLP-DEBUG: [Paso 2] Verificando cola NEW...");
                t_pcb* pcb_new = peek_lista_new();

                if(pcb_new == NULL) {
                    log_info(logger_kernel, "PLP-DEBUG: [Paso 2] Cola NEW está vacía. Terminando ciclo de planificación y esperando nueva señal.");
                    break;
                }
                
                log_info(logger_kernel, "PLP-DEBUG: [Paso 2] Proceso PID %d encontrado en NEW. Intentando crear en Memoria...", pcb_new->pid);
                if (solicitar_creacion_proceso(pcb_new)) {
                    log_info(logger_kernel, "PLP-DEBUG: [Acción] Memoria aceptó PID %d. Moviendo de NEW a READY.", pcb_new->pid);
                    pcb_new = obtener_pcb_de_lista_new();
                    mover_a_ready(pcb_new);
                    //sem_post(&sem_largo_plazo);
                    continue; // Vuelvo al inicio del ciclo
                } else {
                    log_warning(logger_kernel, "PLP-DEBUG: [Acción] Memoria rechazó PID %d (sin espacio).", pcb_new->pid);
                    
                    //Espero a que memoria libere memoria
                    log_info(logger_kernel, "PLP-DEBUG: [Bloqueo] Planificador en espera de memoria libre (sem_memoria_disponible)...");
                    sem_wait(&sem_memoria_disponible);
                    log_debug(logger_kernel, "[PLP] Señal de sem_memoria_disponible recibida."); // DEBUG_LOG
                    log_info(logger_kernel, "PLP-DEBUG: [Señal Recibida] Memoria ha liberado espacio. Reintentando ciclo.");
                    continue;
                }
        }
    return NULL;
    }
}
