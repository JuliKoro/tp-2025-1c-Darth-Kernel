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
#include <semaphore.h>

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                    Gobales

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
#define GRADO_MULTIPROGRAMACION_MAXIMO 3
extern u_int32_t grado_multiprogramacion;
//Esto no se si hace falta usarlo

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Estructuras usadas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/**
 * @brief Estructura que representa una instancia de IO
 * 
 * @param socket_io: El socket de la conexion lo guardamos para poder enviar las peticiones al modulo.
 * @param pid_actual: El PID del proceso actual que esta esperando la respuesta del modulo IO. -1 si esta libre
 */
typedef struct {
    int socket_io; //El socket de la conexion lo guardamos para poder enviar las peticiones al modulo.
    int pid_actual; //El PID del proceso actual que esta esperando la respuesta del modulo IO. -1 si esta libre
} t_instancia_io;


/**
 * @brief Estructura que representa un modulo IO
 * 
 * @param nombre_io: El nombre del modulo IO
 * @param instancias_io: Lista con las instancias disponibles del modulo IO. Cada elemento de la lista es un t_instancia_io, que contiene
 * el socket y el pid actual de cada instancia.
 * @param instancias_disponibles: Cantidad de instancias disponibles del modulo IO.
 */
typedef struct {
    char* nombre_io;
    t_list* instancias_io; //Lista con las instancias disponibles del modulo IO. Contiene el socket y el pid actual de cada instancia.
    int instancias_disponibles; //Cantidad de instancias disponibles del modulo IO.
} t_io;

/**
 * @brief Estructura que representa un proceso bloqueado esperando la respuesta del modulo IO
 * 
 * @param pid: El PID del proceso
 * @param nombre_io: El nombre del IO que el proceso esta esperando
 */


/**
 * @brief Enum que representa los algoritmos de planificacion
 * 
 * @param FIFO: Planificacion FIFO (para ambos planificadores)
 * @param PMCP: Planificacion PMCP (para el planificador largo plazo)
 * @param SJF_SIN_DESALOJO: Planificacion SJF sin desalojo (para el planificador corto plazo)
 * @param SFJ_CON_DESALOJO: Planificacion SFJ con desalojo (para el planificador corto plazo)
 */
typedef enum {
    FIFO,
    PMCP,
    SJF_SIN_DESALOJO,
    SFJ_CON_DESALOJO
} algoritmos_de_planificacion;

/**
 * @brief Estructura que representa un proceso bloqueado esperando la respuesta del modulo IO
 * 
 * @param pid: El PID del proceso
 * @param nombre_io: El nombre del IO que el proceso esta esperando
 */
typedef struct {
    int pid;
    char* nombre_io;
} t_blocked_io;


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Semaforos

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

extern pthread_mutex_t mutex_io;
extern pthread_mutex_t mutex_cola_new;
extern pthread_mutex_t mutex_cola_blocked;
extern pthread_mutex_t mutex_cola_ready;
extern pthread_mutex_t mutex_cola_exit;
extern pthread_mutex_t mutex_cola_executing;
extern pthread_mutex_t mutex_cola_susp_ready;
extern pthread_mutex_t mutex_cola_susp_blocked;
extern pthread_mutex_t mutex_pid_counter;
extern pthread_mutex_t mutex_grado_multiprogramacion;
extern sem_t sem_procesos_en_new;


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Colas Y listas de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

extern t_queue* cola_new;
extern t_queue* cola_ready;
extern t_queue* cola_exit;
extern t_queue* cola_blocked;
extern t_queue* cola_executing;
extern t_queue* cola_susp_ready;
extern t_queue* cola_susp_blocked;
extern t_queue* cola_susp_blocked_io; //Esta cola debe contener structs de t_blocked_io

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
 * @brief Recibe un string que representa el algoritmo de planificacion, viene del archivo de configs. Devuelve el enum correspondiente
 * al algoritmo de planificacion. 
 * 
 * @param algoritmo: El string que representa el algoritmo de planificacion
 * @return El enum correspondiente al algoritmo de planificacion
 */
algoritmos_de_planificacion obtener_algoritmo_de_planificacion(char* algoritmo);

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
void inicializar_colas_y_sem();

/**
 * @brief Recibe un mensaje de la CPU, un paquete. Interpreta el codigo de operacion del paquete y llama a la funcion que corresponda.
 * 
 * Esta función recibe un socket de la CPU y recibe un paquete
 * 
 */
int recibir_mensaje_cpu (int socket_cpu);





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






            /*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                                               SYSCALLS

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                funciones auxiliares para las syscalls

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Busca un IO por su nombre
 * 
 * @param nombre_buscado: El nombre del IO a buscar
 * @return El IO encontrado o NULL si no existe
 */
t_io* buscar_io_por_nombre(char* nombre_buscado);


/**
 * @brief Recibe una syscall en formato string, lee hasta el primer espacio. Interpreta el tipo de syscall.ADJ_FREQUENCY
 * Y llama a la funcion de la syscall que corresponda, con sus parametros.
 * 
 * @param syscall: La syscall a manejar
 * @return El resultado de la syscall
 */
int manejar_syscall(char* syscall);


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones que ejecutan la syscall propiamente

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/**
 * @brief Inicializa un proceso. Esta syscall se llama cuando CPU ejecute la instruccion INIT_PROC
 * 
 * @param archivo_pseudocodigo: El archivo de pseudocódigo del proceso
 * @param tamanio_proceso: El tamaño del proceso
 */
int init_proc(char* archivo_pseudocodigo, int tamanio_proceso);


/**
 * @brief Ejecuta la syscall de IO
 * 
 * @param nombre_io: El nombre del IO
 * @param tiempo_io: El tiempo de IO en milisegundos
 */
int io (char* nombre_io, u_int32_t tiempo_io);



#endif
