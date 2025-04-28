#include "logger.h"


t_log* iniciar_logger(char* archivo_log, char* nombre_modulo) {
	t_log* nuevo_logger = log_create(archivo_log, nombre_modulo, true, LOG_LEVEL_INFO);
    if (nuevo_logger == NULL) {
        fprintf(stderr, "Error fatal: No se pudo crear el logger para el módulo %s\n", nombre_modulo);
        exit(EXIT_FAILURE);
    }
    return nuevo_logger;
}

void iniciar_logger_global(t_log** destino, const char* path, const char* nombre_modulo){
    *destino = iniciar_logger(path, nombre_modulo);
    
    if(*destino == NULL){
        log_error(*destino, "No se pudo crear el logger para el módulo %s", nombre_modulo);
        fprintf(stderr, "Error fatal: No se pudo crear el logger para el módulo %s\n", nombre_modulo);
        exit(EXIT_FAILURE);
    }
}