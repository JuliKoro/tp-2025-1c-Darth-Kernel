#ifndef MEMORIA_LOG_H_
#define MEMORIA_LOG_H_

#include <commons/log.h>
#include "memoria-configs.h"

//Logger global que sera usado en todo el modulo
extern t_log* logger_memoria;

/**
 * @brief Inicializa el logger de la memoria
 * 
 * Esta función no recibe parámetros. Crea un logger con el nivel de log definido en el archivo de configuración
 *
 */ 
void inicializar_logger_memoria();

/**
 * @brief Destruye el logger de la memoria
 * 
 * Esta función no recibe parámetros. Destruye el logger de la memoria
 * 
 */  
void destruir_logger_memoria();

#endif