#ifndef PLANIFICACION_H
#define PLANIFICACION_H

// System includes
#include <string.h>

// External libraries
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

// Local includes
#include <utils/pcb.h>
#include "kernel-conexiones.h"


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                    Gobales

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#define GRADO_MULTIPROGRAMACION_MAXIMO 3
extern u_int32_t grado_multiprogramacion;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Estructuras usadas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

typedef struct {
    char* nombre_io;
    int socket_io; //El socket de la conexion lo guardamos para poder enviar las peticiones al modulo.
    t_queue* cola_blocked_io; //Esta es la cola de procesos que estan bloqueados esperando la respuesta del modulo IO, contiene los PIDs
    int pid_actual; //El PID del proceso actual que esta esperando la respuesta del modulo IO.
    pthread_mutex_t mutex_cola_blocked_io; //Mutex para la cola de procesos bloqueados, ya que vamos a modificar la cola
} t_io;

typedef enum {
    FIFO,
    PMCP
} algoritmo_largo_plazo;

typedef enum {
    FIFOX,
    SJF_SIN_DESALOJO,
    SFJ_CON_DESALOJO
} algoritmo_corto_plazo;


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Semaforos

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

extern pthread_mutex_t mutex_io;
extern pthread_mutex_t mutex_cola_new;
extern pthread_mutex_t mutex_cola_blocked;
extern pthread_mutex_t mutex_cola_ready;
extern pthread_mutex_t mutex_cola_exit;
extern pthread_mutex_t mutex_cola_executing;
extern pthread_mutex_t mutex_pid_counter;
extern pthread_mutex_t mutex_grado_multiprogramacion;


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Colas Y listas de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

extern t_queue* cola_new;
extern t_queue* cola_ready;
extern t_queue* cola_exit;
extern t_queue* cola_blocked;
extern t_queue* cola_executing;

extern t_list* lista_io;


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para asignar PIDs

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

u_int32_t obtener_pid_siguiente();
u_int32_t obtener_pid_actual();

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/**
 * @brief Planifica el proceso inicial
 * 
 * Esta función recibe un archivo de pseudocódigo y un tamaño de proceso, que son cargados desde el main, por parametros
 * Crea un pcb, lo agrega a la cola new y lo loggea
 * 
 * @param archivo_pseudocodigo: Nombre del archivo de pseudocódigo
 * @param tamanio_proceso: Tamaño del proceso en memoria
 * 
 */
void planificar_proceso_inicial(char* archivo_pseudocodigo, u_int32_t tamanio_proceso);


/**
 * @brief Inicializa las colas globales
 * 
 * Esta función no recibe parámetros. Crea las colas new, ready, exit, blocked y executing
 * 
 */
void inicializar_colas();

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para agregar a las colas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Agrega un pcb a la cola new
 * 
 * Esta función recibe un pcb y lo agrega a la cola new
 * 
 */
void agregar_pcb_a_cola_new(t_pcb* pcb);

/**
 * @brief Agrega un pcb a la cola ready
 * 
 * Esta función recibe un pcb y lo agrega a la cola ready
 * 
 */
void agregar_pcb_a_cola_ready(t_pcb* pcb);

/**
 * @brief Agrega un pcb a la cola blocked
 * 
 * Esta función recibe un pcb y lo agrega a la cola blocked
 * 
 */
void agregar_pcb_a_cola_blocked(t_pcb* pcb);


/**
 * @brief Agrega un pcb a la cola exit
 * 
 * Esta función recibe un pcb y lo agrega a la cola exit
 * 
 */
void agregar_pcb_a_cola_exit(t_pcb* pcb);

/**
 * @brief Agrega un pcb a la cola executing
 * 
 * Esta función recibe un pcb y lo agrega a la cola executing
 * 
 */
void agregar_pcb_a_cola_executing(t_pcb* pcb);

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para peekear

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Obtiene un pcb de la cola new
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la cola new
 * 
 */
t_pcb* peek_cola_new();

/**
 * @brief Obtiene un pcb de la cola ready
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la cola ready
 * 
 */
t_pcb* peek_cola_ready();

/**
 * @brief Obtiene un pcb de la cola blocked
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la cola blocked
 * 
 */
t_pcb* peek_cola_blocked();

/**
 * @brief Obtiene un pcb de la cola exit
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la cola exit
 * 
 */
t_pcb* peek_cola_exit();

/**
 * @brief Obtiene un pcb de la cola executing
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la cola executing
 * 
 */
t_pcb* peek_cola_executing();

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para obtener pcb de las colas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Obtiene un pcb de la cola new
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la cola new
 * 
 */
t_pcb* obtener_pcb_de_cola_new();

/**
 * @brief Obtiene un pcb de la cola ready
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la cola ready
 * 
 */
t_pcb* obtener_pcb_de_cola_ready();


/**
 * @brief Obtiene un pcb de la cola executing
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la cola executing
 * 
 */
t_pcb* obtener_pcb_de_cola_executing();

/**
 * @brief Obtiene un pcb de la cola blocked
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la cola blocked
 * 
 */
t_pcb* obtener_pcb_de_cola_blocked();

/**
 * @brief Obtiene un pcb de la cola exit
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la cola exit
 * 
 */
t_pcb* obtener_pcb_de_cola_exit();


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para planificacion largo plazo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Mueve los procesos que han terminado a la cola exit
 * 
 * Esta función no recibe parámetros. Mueve los procesos que han terminado a la cola exit
 * 
 */
void mover_procesos_terminados();

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para planificacion corto plazo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Aumenta el grado de multiprogramacion
 * 
 * Esta función no recibe parámetros. Aumenta el grado de multiprogramacion
 * 
 */
void aumentar_grado_multiprogramacion();

/**
 * @brief Disminuye el grado de multiprogramacion
 * 
 * Esta función no recibe parámetros. Disminuye el grado de multiprogramacion
 * 
 */
void disminuir_grado_multiprogramacion();

/**
 * @brief Comprueba si el grado de multiprogramacion es el maximo
 * 
 * Esta función no recibe parámetros. Comprueba si el grado de multiprogramacion es el maximo
 * 
 */
bool comprobar_grado_multiprogramacion_maximo();

#endif
