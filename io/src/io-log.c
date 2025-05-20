#include "io-log.h"

t_log* logger_io;

void inicializar_logger_io() {
    logger_io = log_create("io.log", "IO", true, log_level_from_string(io_configs.loglevel));
}   

void destruir_logger_io() {
    //Chequeo que el logger exista y lo destruyo
    if (logger_io != NULL) {
        log_destroy(logger_io);
    }
}