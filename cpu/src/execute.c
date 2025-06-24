#include "execute.h"

// Implementación de la función execute
int execute(instruccion_decodificada* instruccion, int socket_memoria) {
    // Log de inicio de ejecución
    log_info(logger_cpu, "Ejecutando instrucción: Tipo=%d, Dirección=%d", instruccion->tipo, instruccion->direccion);

    switch (instruccion->tipo) {
        case WRITE:
            log_info(logger_cpu, "## PID: %d - Ejecutando: WRITE - %d %s", instruccion->pid, instruccion->direccion, instruccion->datos);
            // Lógica para escribir en memoria
            // escribir_en_memoria(socket_memoria, instruccion->direccion, instruccion->datos);
            break;

        case READ:
            log_info(logger_cpu, "## PID: %d - Ejecutando: READ - %d %s", instruccion->pid, instruccion->direccion, instruccion->tamanio);            log_info(logger_cpu, "Ejecutando READ en dirección: %d, Tamaño: %d", instruccion->direccion, instruccion->tamanio);
            // Lógica para leer de memoria
            // leer_de_memoria(socket_memoria, instruccion->direccion, instruccion->tamanio);
            break;

        case GOTO:
            log_info(logger_cpu, "## PID: %d - Ejecutando: GOTO - %d", instruccion->pid, instruccion->pc_destino);
            // Actualizar el PC a la dirección de destino
            // pc_actual = instruccion->pc_destino;
            break;

        case IO:
            log_info(logger_cpu, "## PID: %d - Ejecutando: IO - %s %d", instruccion->pid, instruccion->dispositivo_io, instruccion->tiempo_io);
            // Lógica para manejar operaciones de entrada/salida
            // manejar_io(instruccion->dispositivo_io, instruccion->tiempo_io);
            break;

        case INIT_PROC:
            log_info(logger_cpu, "## PID: %d - Ejecutando: INIT_PROC - %s", instruccion->pid, instruccion->archivo_proceso);
            // Lógica para inicializar un proceso
            // inicializar_proceso(instruccion->archivo_proceso);
            break;

        case DUMP_MEMORY:
            log_info(logger_cpu, "## PID: %d - Ejecutando: DUMP_MEMORY", instruccion->pid);
            // Lógica para volcar la memoria
            // volcar_memoria();
            break;

        case INSTRUCCION_DESCONOCIDA:
            log_error(logger_cpu, "## PID: %d - Instrucción desconocida recibida.", instruccion->pid);
            return -1; // Retornar un código de error si la instrucción no es válida

        default:
            log_error(logger_cpu, "## PID: %d - Instrucción no reconocida: Tipo=%d", instruccion->pid, instruccion->tipo);
            return -1; // Retornar un código de error si la instrucción no es válida
    }
    
    return 0; // Retorna 0 para indicar éxito
}
