#ifndef KERNEL_LOG_H
#define KERNEL_LOG_H

#include <commons/log.h>

#include "kernel-configs.h"
#include <utils/pcb.h>

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


//Funciones de loggeos minimos y obligatorios

/**
 * @brief Loggea el fin de un proceso
 * 
 * @param pcb: Puntero al PCB del proceso
 */
void log_fin_io(t_pcb* pcb);


/**
 * @brief Loggea el desalojo de un proceso
 * 
 * @param pcb: Puntero al PCB del proceso
 */
void log_desalojo(t_pcb* pcb);

/**
 * @brief Loggea las metricas de estado de un proceso
 * 
 * @param pcb: Puntero al PCB del proceso
 * @param estado: El estado del proceso
 */

void log_metricas_estado(t_pcb* pcb);


/**
 * @brief Loggea la creacion de un proceso
 * 
 * @param pcb: Puntero al PCB del proceso
 */
void log_creacion_proceso(t_pcb* pcb);


/**
 * @brief Loggea la solicitud de una syscall
 * 
 * @param pcb: Puntero al PCB del proceso
 * @param syscall: La syscall solicitada, el string con el nombre de la syscall
 */
void log_solicitud_syscall(u_int32_t pid, char* syscall);


/**
 * @brief Loggea el fin de un proceso
 * 
 * @param pcb: Puntero al PCB del proceso
 */
void log_fin_proceso(t_pcb* pcb);


/**
 * @brief Loggea el cambio de estado de un proceso
 * 
 * @param pcb: Puntero al PCB del proceso
 * @param estado_anterior: El estado anterior del proceso
 * @param estado_nuevo: El estado nuevo del proceso
 */
void log_cambio_estado(t_pcb* pcb, estado_pcb estado_anterior, estado_pcb estado_nuevo);


/**
 * @brief Loggea cuando un proceso finaliza IO y pasa a READY
 * 
 * @param pcb: Puntero al PCB del proceso
 */
void log_fin_io(t_pcb* pcb);


/**
 * @brief Loggea el motivo de bloqueo de un proceso
 * 
 * @param pid: El PID del proceso
 * @param nombreio: El nombre del dispositivo IO que bloquea al proceso
 */
void log_motivo_bloqueo(u_int32_t pid, char* nombreio);


/**
 * @brief Convierte un estado de PCB a un string
 * 
 * @param estado: El estado de PCB a convertir
 * @return El string correspondiente al estado de PCB
 */
const char* estado_pcb_to_string(estado_pcb estado);

#endif
