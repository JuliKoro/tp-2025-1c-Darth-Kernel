#ifndef PCB_H
#define PCB_H

#include <commons/collections/list.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>

/**
 * @var pid_counter
 * @brief Contador de PIDs global. Empieza en 0 y se incrementa en 1 cada vez que se crea un nuevo proceso.
 */
extern u_int32_t pid_counter;

/**
 * @enum estado_pcb
 * @brief Enum que representa los posibles estados de un proceso
 * 
 * @param NEW: Estado de nuevo
 * @param READY: Estado de listo
 * @param RUNNING: Estado de corriendo
 * @param BLOCKED: Estado de bloqueado
 * @param EXIT: Estado de finalizado
 */
typedef enum {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    EXIT
} estado_pcb;

/**
 * @struct t_pcb
 * @brief Estructura que representa un PCB (Program Control Block). Tiene los datos minimos por ahora.
 * 
 * @param pid: Identificador único del proceso
 * @param pc: Contador de programa que indica la dirección de la instrucción actual
 * @param metricas_estado: Lista de métricas de estado
 * @param metricas_tiempo: Lista de métricas de tiempo
 * @param archivo_pseudocodigo: Nombre del archivo de pseudocódigo
 * @param tamanio_proceso: Tamaño del proceso en bytes
 * @param estado: Estado actual del proceso
 */
typedef struct t_pcb { 
    u_int32_t pid;
    u_int32_t pc;
    t_list* metricas_estado;
    t_list* metricas_tiempo;
    char* archivo_pseudocodigo;
    u_int32_t tamanio_proceso;
    estado_pcb estado;
} t_pcb;

/**
 * @brief Obtiene el estado del proceso en formato string
 * 
 * @param pcb: Puntero a un t_pcb
 * @return Enum estado_pcb correspondiente
 */
estado_pcb obtener_estado_pcb(t_pcb* pcb);

/**
 * @brief Inicializa un PCB con los datos proporcionados
 * 
 * @param pid: Identificador único del proceso
 * @param archivo_pseudocodigo: Nombre del archivo de pseudocódigo
 * @param tamanio_proceso: Tamaño del proceso en bytes
 * @return Puntero al t_pcb inicializado
 */
t_pcb* inicializar_pcb(u_int32_t pid, char* archivo_pseudocodigo, u_int32_t tamanio_proceso);

#endif
