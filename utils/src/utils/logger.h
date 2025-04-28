#ifndef LOGGER_H
#define LOGGER_H

#include <commons/log.h>
#include <stdlib.h>


/**
* @brief Inicializa un logger que muestra en consola, en modo LOG_LEVEL_INFO
* @param archivo_log El nombnre del archivo
* @param nombre_modulo El nombre del modulo que va aparecer en el archivo log
* @return Devuelve una t_log* logger
*/

t_log* iniciar_logger(char* archivo_log, char* nombre_modulo);


/**
* @brief Recibe la direccion del logger global, crea un logger con los otros parametros, e iguala el logger global a este nuevo creado
* @param destino La direccion del logger global
* @param path El nombre del archivo log que va a ser creado
* @param nombre_moduloe El nombre que va a mostrar en log en cada entrada
* @return Devuelve una t_log* logger
*/
void iniciar_logger_global(t_log** destino, const char* path, const char* nombre_modulo);

#endif