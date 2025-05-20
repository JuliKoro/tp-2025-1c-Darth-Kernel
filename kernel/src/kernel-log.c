#include "kernel-log.h"

t_log* logger_kernel;

void inicializar_logger_kernel() {
    logger_kernel = log_create("kernel.log", "Kernel", true, log_level_from_string(kernel_configs.loglevel));
}

void destruir_logger_kernel() {
    //Chequeo que el logger exista y lo destruyo
    if (logger_kernel != NULL) {
        log_destroy(logger_kernel);
    }
}