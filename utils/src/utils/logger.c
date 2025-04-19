#include "logger.h"


t_log* iniciar_logger(char* archivo_log, char* nombre_modulo) {
	t_log* nuevo_logger = log_create(archivo_log, nombre_modulo, true, LOG_LEVEL_INFO);
    return nuevo_logger;
}