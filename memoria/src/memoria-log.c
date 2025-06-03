#include "memoria-log.h"

t_log* logger_memoria;

void inicializar_logger_memoria() {
    logger_memoria = log_create("memoria.log", "MEMORIA", true, log_level_from_string(memoria_configs.loglevel));
}

void destruir_logger_memoria() {
    //Chequeo que el logger exista y lo destruyo
    if (logger_memoria != NULL) {
        log_destroy(logger_memoria);
    }
}
