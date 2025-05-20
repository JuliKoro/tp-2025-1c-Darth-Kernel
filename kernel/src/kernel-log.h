#ifndef KERNEL_LOG_H
#define KERNEL_LOG_H

#include <commons/log.h>

#include "kernel-configs.h"

//Logger global que sera usado en todo el modulo
extern t_log* logger_kernel;

/**
 * @brief Inicializa el logger del kernel
 * 
 * Esta función no recibe parámetros. Crea un logger con el nivel de log definido en el archivo de configuración
 * 
 */
void inicializar_logger_kernel();

/** 
 * @brief Destruye el logger del kernel
 * 
 * Esta función no recibe parámetros. Destruye el logger del kernel
 * 
 */
void destruir_logger_kernel();

#endif
