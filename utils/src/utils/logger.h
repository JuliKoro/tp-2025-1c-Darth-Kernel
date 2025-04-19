#ifndef LOGGER_H
#define LOGGER_H

#include <commons/log.h>



/**
* @brief Inicializa un logger que muestra en consola, en modo LOG_LEVEL_INFO
* @param archivo_log El nombnre del archivo
* @param nombre_modulo El nombre del modulo que va aparecer en el archivo log
* @return Devuelve una t_log* logger
*/

t_log* iniciar_logger(char* archivo_log, char* nombre_modulo);

#endif