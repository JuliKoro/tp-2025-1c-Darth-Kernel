#include "ciclo-instruccion.h"

void ciclo_instruccion(t_proceso_cpu* proceso, int socket_memoria, int socket_kernel_dispatch, int socket_kernel_interrupt){
    PC = proceso->pc; // Asigno el PC pasado desde Kernel al PC global de CPU

    while(1){ // loop ciclo de instruccion
        
        char* paquete_instruccion = fetch(proceso, socket_memoria); // ETAPA FETCH

        instruccion_decodificada* instruccion_decodificada = decodificar_instruccion(paquete_instruccion, proceso->pid); // ETAPA DECODE

        // ETAPA EXECUTE
        execute(instruccion_decodificada, proceso, socket_memoria, socket_kernel_dispatch);

        // CHECK INTERRUPT
        if (check_interrupt(proceso, socket_kernel_interrupt)) {
            interrupcion->pc = PC; // Actualizo el PC del struct de proceso (PID + PC)
            enviar_devolucion_interrupcion(socket_kernel_dispatch);
            break; // Salir del ciclo en caso de interrupción
        }

        // LIMPIAR INSTRUCCIONES
        destruir_instruccion(instruccion_decodificada);
        free(paquete_instruccion);
    }
    
}

// ETAPA FETCH
char* fetch(t_proceso_cpu* proceso, int socket_memoria){
    proceso->pc = PC; // Actualiza el PC del struct de proceso para enviar a memoria
    log_info(logger_cpu, "## PID: %d - FETCH - Program Counter: %d", proceso->pid, proceso->pc);

    // Verificar conexión
    if (socket_memoria < 0) {
        log_error(logger_cpu, "Error: Socket de memoria no válido.");
        return NULL;
    }
    
    //Serializo la instruccion y envio
    t_buffer* buffer = serializar_proceso_cpu(proceso);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_PROCESO_CPU, buffer);
    enviar_paquete(socket_memoria, paquete);
    
    // Recibe la instruccion de Memoria :)
    char* instruccion_identificada = recibir_mensaje(socket_memoria);
    printf("Instruccion recibida:\n%s\n", instruccion_identificada);

    // Validar la instrucción recibida
    if (instruccion_identificada == NULL) {
        log_error(logger_cpu, "Error: No se recibió ninguna instrucción de memoria.");
        return NULL; // Manejar el error adecuadamente
    }

    // Log de instrucción recibida
    log_debug(logger_cpu, "Instrucción recibida: %s", instruccion_identificada);

    return instruccion_identificada;
}

// ETAPA DECODE
instruccion_decodificada* decodificar_instruccion(char* instruccion_str, uint32_t pid) {

    log_info(logger_cpu, "## PID: %d - DECODE: %s",pid, instruccion_str);
   
    instruccion_decodificada* instruccion = malloc(sizeof(instruccion_decodificada));
    memset(instruccion, 0, sizeof(instruccion_decodificada));
    
    instruccion->pid = pid;

    char** tokens = string_split(instruccion_str, " "); // Separar la instrucción en tokens (partes)

    // Identificación del tipo
    if(strcmp(tokens[0], "NOOP") == 0) {
        instruccion->tipo = NOOP;
        instruccion->requiere_traduccion = false; 
    }
    else if(strcmp(tokens[0], "WRITE") == 0) { // Segun el tipo, parsea los parámetros adicionales
        instruccion->tipo = WRITE;
        instruccion->direccion = atoi(tokens[1]);
        instruccion->datos = strdup(tokens[2]);
        instruccion->requiere_traduccion = true; 
    }
    else if(strcmp(tokens[0], "READ") == 0) {
        instruccion->tipo = READ;
        instruccion->direccion = atoi(tokens[1]);
        instruccion->tamanio = atoi(tokens[2]);
        instruccion->requiere_traduccion = true; 
    }
    else if(strcmp(tokens[0], "GOTO") == 0) {
        instruccion->tipo = GOTO;
        instruccion->pc_destino = atoi(tokens[1]);
        instruccion->requiere_traduccion = false; 
    }
    else if(strcmp(tokens[0], "IO") == 0) {
        instruccion->tipo = IO;
        instruccion->dispositivo_io = strdup(tokens[1]);
        instruccion->tiempo_io = atoi(tokens[2]);
    }
    else if(strcmp(tokens[0], "INIT_PROC") == 0) {
        instruccion->tipo = INIT_PROC;
        instruccion->archivo_proceso = strdup(tokens[1]);
        instruccion->tamanio = atoi(tokens[2]);
    }
    else if(strcmp(tokens[0], "DUMP_MEMORY") == 0) {
        instruccion->tipo = DUMP_MEMORY;
    }
    else if(strcmp(tokens[0], "EXIT") == 0) {
        instruccion->tipo = EXIT_INSTR;
    }
    else {
        instruccion->tipo = INSTRUCCION_DESCONOCIDA;
    }
    
    // Liberar memoria de los tokens
    for(int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
    
    return instruccion;
}

// CHECK INTERRUPT
bool check_interrupt(t_proceso_cpu* proceso, int socket_kernel_interrupt) {
    if (IF == 1) {
        // Hay una Interrupcion
        if (interrupcion->pid == proceso->pid) {
            log_debug(logger_cpu, "Se ha detectado una interrupcion para el proceso %d", proceso->pid);
            return true;
        } else {
            log_error(logger_cpu, "Interrupcion invalida. No corresponde al PID: %d", proceso->pid);
            return false;  
        }
    } else {
        // No hay interrupciones
        log_debug(logger_cpu, "No se ha detectado ninguna interrupcion para el proceso");
        return false;
    }
}

void enviar_devolucion_interrupcion(int socket_kernel_dispatch){
    t_buffer* buffer = serializar_interrupcion(interrupcion);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_INTERRUPCION, buffer);
    if(enviar_paquete(socket_kernel_dispatch, paquete) == -1){
        log_error(logger_cpu, "Error: No se pudo enviar la devulucion de la interrupcion a kernel");
        return;
    }  
}

void destruir_instruccion(instruccion_decodificada* instruccion) {
    if(instruccion->datos != NULL) free(instruccion->datos);
    if(instruccion->dispositivo_io != NULL) free(instruccion->dispositivo_io);
    if(instruccion->archivo_proceso != NULL) free(instruccion->archivo_proceso);
    free(instruccion);
}