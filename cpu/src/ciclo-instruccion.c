#include "ciclo-instruccion.h"

void ciclo_instruccion(int pid, uint32_t pc, int socket_memoria){
    while(1){ // loop ciclo de instruccion
        
        char* paquete_instruccion = fetch(pid, pc, socket_memoria); // ETAPA FETCH

        instruccion_decodificada* instruccion = decodificar_instruccion(paquete_instruccion, pc); // ETAPA DECODE

        check_interrupt();


        // LIMPIAR INSTRUCCIONES
        destruir_instruccion(instruccion);
        free(paquete_instruccion);
    }
    
}

// ETAPA FETCH
char* fetch(uint32_t pid, uint32_t pc, int socket_memoria){
    // Verificar conexión
    if (socket_memoria < 0) {
        log_error(logger_cpu, "Error: Socket de memoria no válido.");
        return NULL;
    }

    //Le pido a memoria la instruccion
    t_paquete* paquete = crear_paquete();
    agregar_a_paquete(paquete, pid);
    agregar_a_paquete(paquete, pc);
    enviar_paquete(paquete, socket_memoria);
    free(paquete);
    
    // Recibe la instruccion de Memoria :)
    char* instruccion = recibir_mensaje(socket_memoria);
    printf("Instruccion recibida:\n%s\n", instruccion);

    // Validar la instrucción recibida
    if (instruccion == NULL) {
        log_error(logger_cpu, "Error: No se recibió ninguna instrucción de memoria.");
        return NULL; // Manejar el error adecuadamente
    }

    // Log de instrucción recibida
    log_info(logger_cpu, "Instrucción recibida: %s", instruccion);

    return instruccion;
}

// ETAPA DECODE
instruccion_decodificada* decodificar_instruccion(char* instruccion_str, uint32_t pc_actual) {

    log_info(logger_cpu, "Decodificando instrucción: %s", instruccion_str);
   
    instruccion_decodificada* instruccion = malloc(sizeof(instruccion_decodificada));
    memset(instruccion, 0, sizeof(instruccion_decodificada));
    
    char** tokens = string_split(instruccion_str, " ");
    
    if(strcmp(tokens[0], "NOOP") == 0) {
        instruccion->tipo = NOOP;
    }
    else if(strcmp(tokens[0], "WRITE") == 0) {
        instruccion->tipo = WRITE;
        instruccion->direccion = atoi(tokens[1]);
        instruccion->datos = strdup(tokens[2]);
    }
    else if(strcmp(tokens[0], "READ") == 0) {
        instruccion->tipo = READ;
        instruccion->direccion = atoi(tokens[1]);
        instruccion->tamanio = atoi(tokens[2]);
    }
    else if(strcmp(tokens[0], "GOTO") == 0) {
        instruccion->tipo = GOTO;
        instruccion->pc_destino = atoi(tokens[1]);
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
        instruccion->tipo = EXIT;
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

void destruir_instruccion(instruccion_decodificada* instruccion) {
    if(instruccion->datos != NULL) free(instruccion->datos);
    if(instruccion->dispositivo_io != NULL) free(instruccion->dispositivo_io);
    if(instruccion->archivo_proceso != NULL) free(instruccion->archivo_proceso);
    free(instruccion);
}
