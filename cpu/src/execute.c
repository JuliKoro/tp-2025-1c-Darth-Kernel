#include "execute.h"

// Implementación de la función execute
int execute(instruccion_decodificada* instruccion, t_proceso_cpu* proceso, int socket_memoria, int socket_kernel_dispatch) {
    // Log de inicio de ejecución
    char* parametros = parametros_str(instruccion);
    log_info(logger_cpu, "## PID: %d - Ejecutando: %s - %s", instruccion->pid, instruccion_str(instruccion->tipo), parametros);

    switch (instruccion->tipo) {
        case NOOP:
            // NOOP solo consume tiempo, no hace nada
            sleep(1); // Simula un ciclo de instrucción de 1 segundo
            PC++; // Incremento el PC en 1 (actualiza el PC global)
            break;
        case WRITE:
            // Lógica para escribir en memoria
            // escribir_en_memoria(socket_memoria, instruccion->direccion, instruccion->datos);
            PC++;
            break;

        case READ:
            // Lógica para leer de memoria
            // leer_de_memoria(socket_memoria, instruccion->direccion, instruccion->tamanio);
            PC++;
            break;

        case GOTO:
            // Actualizar el PC global a la dirección de destino
            PC = instruccion->pc_destino;
            break;

        case IO: // SYSCALL (Dispositivo, Tiempo)
            // manejar operaciones de entrada/salida
            enviar_syscall(instruccion, socket_kernel_dispatch);
            PC++;
            break;

        case INIT_PROC: // SYSCALL (Archivo de instrucciones, Tamaño)
            // inicializar un proceso
            enviar_syscall(instruccion, socket_kernel_dispatch);
            PC++;
            break;

        case DUMP_MEMORY: // SYSCALL
            // volcar la memoria
            enviar_syscall(instruccion, socket_kernel_dispatch);
            PC++;
            break;

        case EXIT_INSTR: // SYSCALL
            enviar_syscall(instruccion, socket_kernel_dispatch);
            PC++;
            break;

        case INSTRUCCION_DESCONOCIDA:
            log_error(logger_cpu, "## PID: %d - Instrucción desconocida recibida.", instruccion->pid);
            PC++;
            return -1; // Retornar un código de error si la instrucción no es válida

        default:
            log_error(logger_cpu, "## PID: %d - Instrucción no reconocida: Tipo=%d", instruccion->pid, instruccion->tipo);
            return -1; // Retornar un código de error si la instrucción no es válida
    }
    
    free(parametros);
    return 0; // Retorna 0 para indicar éxito
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                    Agrego funciones de syscalls relevantes a kernel, por ahora solo envia la syscall.
                                    Falta recibir la respuesta segun corresponda en cada syscall.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

int enviar_syscall(instruccion_decodificada* instruccion, int socket_kernel_dispatch){
    //Recibo instruccion decodificada, me fijo el tipo. Segun el tipo, llamo a la funcion que corresponde
    tipo_instruccion tipo = instruccion->tipo;
    switch(tipo){
        case IO:
            enviar_syscall_io(instruccion, socket_kernel_dispatch);
            break;
        case INIT_PROC:
            enviar_syscall_init_proc(instruccion, socket_kernel_dispatch);
            break;
        case DUMP_MEMORY:
            enviar_syscall_dump_memory(instruccion, socket_kernel_dispatch);
            break;
        case EXIT_INSTR:
            enviar_syscall_exit(instruccion, socket_kernel_dispatch);
            break;
        default:
            printf("Error: syscall desconocida\n");
            return -1;
    }
    return 0;
}

void enviar_syscall_io(instruccion_decodificada* instruccion, int socket_kernel_dispatch){
    t_syscall* syscall = malloc(sizeof(t_syscall));
    syscall->syscall = malloc(sizeof(char) * 100);
    //Armo el string de la syscall. Tiene el formato: "IO <dispositivo_io> <tiempo_io>"
    sprintf(syscall->syscall, "IO %s %d", instruccion->dispositivo_io, instruccion->tiempo_io);
    //Asigno el pid del proceso que llamo la syscall
    syscall->pid = instruccion->pid;
    //Serializo la syscall y armo un paquete, luego lo envio
    t_buffer* buffer = serializar_syscall(syscall);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_SYSCALL, buffer);
    if(enviar_paquete(socket_kernel_dispatch, paquete) == -1){
        log_error(logger_cpu, "Error: No se pudo enviar la syscall a kernel");
        return;
    }
    //Libero la memoria de la syscall y el buffer
    free(syscall->syscall);
    free(syscall);
}

void enviar_syscall_init_proc(instruccion_decodificada* instruccion, int socket_kernel_dispatch){
    t_syscall* syscall = malloc(sizeof(t_syscall));
    syscall->syscall = malloc(sizeof(char) * 100);
    //Armo el string de la syscall. Tiene el formato: "INIT_PROC <archivo_proceso> <tamanio>"
    sprintf(syscall->syscall, "INIT_PROC %s %d", instruccion->archivo_proceso, instruccion->tamanio);
    //Asigno el pid del proceso que llamo la syscall
    syscall->pid = instruccion->pid;
    //Serializo la syscall y armo un paquete, luego lo envio
    t_buffer* buffer = serializar_syscall(syscall);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_SYSCALL, buffer);
    if(enviar_paquete(socket_kernel_dispatch, paquete) == -1){
        log_error(logger_cpu, "Error: No se pudo enviar la syscall a kernel");
        return;
    }
    //Libero la memoria de la syscall y el buffer despues de enviarlo
    free(syscall->syscall);
    free(syscall);
}

void enviar_syscall_dump_memory(instruccion_decodificada* instruccion, int socket_kernel_dispatch){
    t_syscall* syscall = malloc(sizeof(t_syscall));
    syscall->syscall = malloc(sizeof(char) * 100);
    sprintf(syscall->syscall, "DUMP_MEMORY");
    syscall->pid = instruccion->pid;
    //Serializo la syscall y armo un paquete, luego lo envio
    t_buffer* buffer = serializar_syscall(syscall);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_SYSCALL, buffer);
    if(enviar_paquete(socket_kernel_dispatch, paquete) == -1){
        log_error(logger_cpu, "Error: No se pudo enviar la syscall a kernel");
        return;
    }
    //Libero la memoria de la syscall y el buffer despues de enviarlo
    free(syscall->syscall);
    free(syscall);
}

void enviar_syscall_exit(instruccion_decodificada* instruccion, int socket_kernel_dispatch){
    t_syscall* syscall = malloc(sizeof(t_syscall));
    syscall->syscall = malloc(sizeof(char) * 100);
    sprintf(syscall->syscall, "EXIT");
    syscall->pid = instruccion->pid;
    //Serializo la syscall y armo un paquete, luego lo envio
    t_buffer* buffer = serializar_syscall(syscall);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_SYSCALL, buffer);
    if(enviar_paquete(socket_kernel_dispatch, paquete) == -1){
        log_error(logger_cpu, "Error: No se pudo enviar la syscall a kernel");
        return;
    }   
    //Libero la memoria de la syscall y el buffer despues de enviarlo
    free(syscall->syscall);
    free(syscall);
}