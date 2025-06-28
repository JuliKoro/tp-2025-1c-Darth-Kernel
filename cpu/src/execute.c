#include "execute.h"

// Implementación de la función execute
int execute(instruccion_decodificada* instruccion, int socket_memoria) {
    // Log de inicio de ejecución
    char* parametros = parametros_str(instruccion);
    log_info(logger_cpu, "## PID: %d - Ejecutando: %s - %s", instruccion->pid, instruccion_str(instruccion->tipo), parametros);

    switch (instruccion->tipo) {
        case NOOP:
            // NOOP solo consume tiempo, no hace nada
            sleep(1); // Simula un ciclo de instrucción de 1 segundo
            pc++; // Incremento el PC en 1 (actualiza el PC global)
        case WRITE:
            // Lógica para escribir en memoria
            // escribir_en_memoria(socket_memoria, instruccion->direccion, instruccion->datos);
            break;

        case READ:
            // Lógica para leer de memoria
            // leer_de_memoria(socket_memoria, instruccion->direccion, instruccion->tamanio);
            break;

        case GOTO:
            // Actualizar el PC global a la dirección de destino
            pc = instruccion->pc_destino;
            break;

        case IO: // SYSCALL (Dispositivo, Tiempo)
            // manejar operaciones de entrada/salida
            // manejar_io(instruccion->dispositivo_io, instruccion->tiempo_io);
            break;

        case INIT_PROC: // SYSCALL (Archivo de instrucciones, Tamaño)
            // inicializar un proceso
            // inicializar_proceso(instruccion->archivo_proceso);
            break;

        case DUMP_MEMORY: // SYSCALL
            // volcar la memoria
            // volcar_memoria();
            break;

        case EXIT: // SYSCALL
            
            break;

        case INSTRUCCION_DESCONOCIDA:
            log_error(logger_cpu, "## PID: %d - Instrucción desconocida recibida.", instruccion->pid);
            return -1; // Retornar un código de error si la instrucción no es válida

        default:
            log_error(logger_cpu, "## PID: %d - Instrucción no reconocida: Tipo=%d", instruccion->pid, instruccion->tipo);
            return -1; // Retornar un código de error si la instrucción no es válida
    }
    
    free(parametros);
    return 0; // Retorna 0 para indicar éxito
}
