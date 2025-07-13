#ifndef PCB_H
#define PCB_H

#include <commons/collections/list.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>

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
 * @param SUSP_BLOCKED: Estado de suspendido y bloqueado
 * @param SUSP_READY: Estado de suspendido y listo
 */
typedef enum {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    SUSP_BLOCKED,
    SUSP_READY, 
    EXIT,
    ESTADOS_COUNT
} estado_pcb;

/**
 * @struct t_metricas_estado
 * @brief Estructura que representa las métricas de estado de un proceso
 * 
 * @param estado: Estado del proceso
 * @param contador: Cantidad de veces que el proceso ha estado en ese estado
 */
typedef struct t_metricas_estado {
    estado_pcb estado;
    int contador;
} t_metricas_estado;

/**
 * @struct t_metricas_tiempo
 * @brief Estructura que representa las métricas de tiempo de un proceso
 * 
 * @param estado: Estado del proceso
 * @param tiempo_acumulado: Tiempo acumulado del proceso en ese estado
 */
typedef struct t_metricas_tiempo {
    estado_pcb estado;
    double tiempo_acumulado;
} t_metricas_tiempo;
/**
 * @struct t_pcb
 * @brief Estructura que representa un PCB (Program Control Block). Tiene los datos minimos por ahora.
 * 
 * @param ult_update: Ultima hora de actualizacion del proceso
 * @param pid: Identificador único del proceso
 * @param pc: Contador de programa que indica la dirección de la instrucción actual
 * @param metricas_estado: Lista de métricas de estado
 * @param metricas_tiempo: Lista de métricas de tiempo
 * @param archivo_pseudocodigo: Nombre del archivo de pseudocódigo
 * @param tamanio_proceso: Tamaño del proceso en bytes
 * @param estado: Estado actual del proceso
 * @param mutex_cambio_estado: Mutex para proteger el cambio de estado
 */
typedef struct t_pcb { 
    struct timeval ult_update;
    u_int32_t pid;
    u_int32_t pc;
    t_metricas_estado metricas_estado[ESTADOS_COUNT];
    t_metricas_tiempo metricas_tiempo[ESTADOS_COUNT];
    char* archivo_pseudocodigo;
    u_int32_t tamanio_proceso;
    estado_pcb estado;
    pthread_mutex_t mutex_cambio_estado;
} t_pcb;




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