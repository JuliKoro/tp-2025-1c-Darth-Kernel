#include "cpu-log.h"

t_log* logger_cpu;

t_log* iniciar_logger_cpu(int id_cpu){
    char log_filename[64]; // Crear nombre de log dinámico (log por cada CPU)
    snprintf(log_filename, sizeof(log_filename), "cpu_%d.log", id_cpu); // formato nombre del archvio 
    t_log* logger_cpu = iniciar_logger(log_filename, "[CPU]"); // crea logger cpu_<id>.log
    if (logger_cpu == NULL) {
        fprintf(stderr, "Error al crear el logger.\n");
        return NULL;
    }
    return logger_cpu;
 }

 void destruir_logger_cpu() {
    //Chequeo que el logger exista y lo destruyo
    if (logger_cpu != NULL) {
        log_destroy(logger_cpu);
    }
}

char* instruccion_str(tipo_instruccion tipo){
    switch (tipo) {
        case NOOP: return "NOOP";
        case WRITE: return "WRITE";
        case READ: return "READ";
        case GOTO: return "GOTO";
        case IO: return "IO";
        case INIT_PROC: return "INIT_PROC";
        case DUMP_MEMORY: return "DUMP_MEMORY";
        case EXIT_INSTR: return "EXIT";
        case INSTRUCCION_DESCONOCIDA: return "INSTRUCCION_DESCONOCIDA";
        default: return "Instrucción no reconocida";
    }
}

char* parametros_str(instruccion_decodificada* instruccion){
    char* parametros = malloc(256); // Asignar memoria para el string de parámetros (LIBERAR)
    if (!parametros) return NULL; // Manejo de error en caso de falla de memoria
    switch (instruccion->tipo) {
        case WRITE:
            snprintf(parametros, 256, "%d %s", instruccion->direccion, instruccion->datos);
            break;
        case READ:
            snprintf(parametros, 256, "%d %d", instruccion->direccion, instruccion->tamanio);
            break;
        case GOTO:
            snprintf(parametros, 256, "%d", instruccion->pc_destino);
            break;
        case IO:
            snprintf(parametros, 256, "%s %d", instruccion->dispositivo_io, instruccion->tiempo_io);
            break;
        case INIT_PROC:
            snprintf(parametros, 256, "%s", instruccion->archivo_proceso);
            break;
        case DUMP_MEMORY:
            snprintf(parametros, 256, " ");
            break;
        case NOOP:
            snprintf(parametros, 256, " ");
            break;
        case EXIT_INSTR:
            snprintf(parametros, 256, " ");
            break;
        case INSTRUCCION_DESCONOCIDA:
            snprintf(parametros, 256, " ");
            break;
        default:
            snprintf(parametros, 256, " ");
            break;
    }
    return parametros;
}
