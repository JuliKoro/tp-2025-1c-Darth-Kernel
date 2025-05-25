#ifndef KERNEL_CONEXIONES_H_
#define KERNEL_CONEXIONES_H_


#include <utils/sockets.h>
#include <utils/configs.h>
#include <utils/logger.h>

#include "kernel-configs.h"
#include "kernel-log.h"

//Conexion con CPU

/**
* @brief Función genérica para iniciar un servidor
* @param puerto Puerto en el que escuchará el servidor
* @param nombre_modulo Nombre del módulo que se conectará
* @param tipo_modulo Tipo de módulo para el handshake
* @return Devuelve socket del cliente para comunicarse
*/
int iniciar_servidor_generico(int puerto, char* nombre_modulo, int tipo_modulo);

/**
* @brief Levanta servidor en el puerto de escucha de dispatch
* @return Devuelve socket del cliente para comunicarse con CPU
*/
int iniciar_servidor_dispatch(void);

/**
* @brief Levanta servidor en el puerto de escucha de interrupciones
* @return Devuelve socket del cliente para comunicarse con CPU-Interrupt
*/
int iniciar_servidor_interrupt(void);
 
/**
* @brief Levanta servidor en el puerto de escucha de IO
* @return Devuelve socket del cliente para comunicarse con IO
*/
int iniciar_servidor_io(void);

/**
* @brief Conecta con la memoria
* @return Devuelve socket del cliente para comunicarse con memoria
*/
int kernel_conectar_a_memoria(void);

/**
* @brief Solicita la creación de un proceso a memoria
* @param pcb: PCB del proceso a crear
* @return Devuelve true si la solicitud es exitosa, false en caso contrario
*/
bool solicitar_creacion_proceso(t_pcb* pcb);

#endif