#ifndef IO_LOG_H
#define IO_LOG_H

#include <commons/log.h>

#include "io-configs.h"

extern t_log* logger_io;

/**
 * @brief Inicializa el logger del io
 * 
 * Esta función no recibe parámetros. Crea un logger con el nivel de log definido en el archivo de configuración
 * 
 */
void inicializar_logger_io();



/**
 * @brief Destruye el logger del io
 * 
 * Esta función no recibe parámetros. Destruye el logger del io
 * 
 */
void destruir_logger_io();



#endif
