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
#include <utils/listas.h>
#include <utils/semaforos.h>
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
    SFJ_CON_DESALOJO,

} algoritmos_de_planificacion;

/**
 * @brief Estructura que representa un proceso bloqueado esperando la respuesta del modulo IO
 * 
 * @param pid: El PID del proceso
 * @param nombre_io: El nombre del IO que el proceso esta esperando
 * @param tiempo_io: El tiempo de IO que el proceso esta esperando
 */
typedef struct {
    u_int32_t pid;
    char* nombre_io;
    u_int32_t tiempo_io;
} t_blocked_io;


/**
 * @brief Estructura que representa una CPU en el kernel
 * 
 * @param socket_cpu_dispatch: El socket de la CPU para el dispatch
 * @param socket_cpu_interrupt: El socket de la CPU para el interrupt
 * @param id_cpu: El ID de la CPU 
 */
typedef struct {
    int socket_cpu_dispatch;
    int socket_cpu_interrupt;
    int id_cpu;
    bool esta_ocupada;
    int pid_actual;
} t_cpu_en_kernel;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones auxiliares a estructuras

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/**
 * @brief Crea un t_blocked_io
 * 
 * @param pid: El pid del pcb a mover
 * @param nombre_io: El nombre del io
 * @param tiempo_io: El tiempo de io
 * @return El t_blocked_io creado
 */
t_blocked_io* crear_blocked_io(u_int32_t pid, char* nombre_io, u_int32_t tiempo_io);

bool comparar_pid(void* elemento, void* pid);


/**
 * @brief Actualiza el estado de un pcb
 * 
 * @param pcb: El puntero al  pcb a actualizar
 * @param estado: El estado del pcb
 * @return 0 si el estado se actualiza correctamente
 */
int actualizar_estado_pcb(t_pcb* pcb, estado_pcb estado);

/**
 * @brief Recibe un socket de una instancia de IO, y busca el proceso en la lista de blocked_io
 * 
 * @param socket_io Socket buscado
 * @return Un t_blocked_io, es un PID del proceso que se bloqueo por esta IO
 */
t_blocked_io* buscar_proceso_en_blocked_io(int socket_io);




/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Semaforos

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

extern pthread_mutex_t mutex_io;
extern pthread_mutex_t mutex_lista_new;
extern pthread_mutex_t mutex_lista_blocked;
extern pthread_mutex_t mutex_lista_ready;
extern pthread_mutex_t mutex_lista_exit;
extern pthread_mutex_t mutex_lista_executing;
extern pthread_mutex_t mutex_pid_counter;
extern pthread_mutex_t mutex_grado_multiprogramacion;
extern pthread_mutex_t mutex_lista_blocked_io;
extern pthread_mutex_t mutex_cpu;
extern pthread_mutex_t mutex_lista_suspblocked;
extern pthread_mutex_t mutex_lista_suspready;


extern sem_t sem_largo_plazo;
extern sem_t sem_corto_plazo;
extern sem_t sem_cpu_disponible;
extern sem_t sem_memoria_disponible;
/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Colas Y listas de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

extern t_list* lista_new;
extern t_list* lista_ready;

extern t_list* lista_exit; //Esto es una lista porque tengo que poder ordenarla y modificarla
extern t_list* lista_blocked; //Esto es una lista porque tengo que poder ordenarla y modificarla
extern t_list* lista_executing; //Esto es una lista porque tengo que poder ordenarla y modificarla
extern t_list* lista_suspready; //Esto es una lista porque tengo que poder ordenarla y modificarla
extern t_list* lista_suspblocked; //Esto es una lista porque tengo que poder ordenarla y modificarla
extern t_list* lista_blocked_io; //Esta cola debe contener structs de t_blocked_io

extern t_list* lista_io;
extern t_list* lista_cpu;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para asignar PIDs

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

u_int32_t obtener_pid_siguiente();
u_int32_t obtener_pid_actual();

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones de planificacion

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Encontra un pcb por su pid
 * 
 * Esta función recibe un elemento y un pid. Compara el pid del elemento con el pid recibido.
 * 
 * @param elemento: El elemento a comparar
 * @param pid_a_comparar: El pid a comparar
 * @return true si el pid del elemento es igual al pid recibido, false en caso contrario
 */
bool _encontrar_pcb_por_pid(void* elemento, void* pid_a_comparar);




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
 * @brief Inicializa las listas globales y los semáforos
 * 
 * Esta función no recibe parámetros. Crea las listas new, ready, exit, blocked y executing
 * 
 */
void inicializar_listas_y_sem();

/**
 * @brief Recibe un mensaje de la CPU, un paquete. Interpreta el codigo de operacion del paquete y llama a la funcion que corresponda.
 * 
 * Esta función recibe un socket de la CPU y recibe un paquete
 * 
 */
int recibir_mensaje_cpu (int socket_cpu);
//Revisar





/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para mover entre estados

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Agrega un pcb a la cola new
 * 
 * Esta función recibe un pcb y lo agrega a la cola new
 * 
 */
void agregar_a_new(t_pcb* pcb);

/**
 * @brief Agrega un pcb a la lista ready
 * 
 * Esta función recibe un pcb y lo agrega a la lista ready
 * NOTA: Para algoritmos como SJF o PMCP, esta función debería modificarse para insertar el PCB de forma ordenada.
 * 
 */
void mover_a_ready(t_pcb* pcb);

/**
 * @brief Mueve un pcb de la lista blocked a la lista exit. Ejecuta los procedimientos que se necesitan cada vez que se mueve un pcb a exit, desde cualquier lado.
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_a_exit (t_pcb* pcb);

/**
 * @brief Mueve un pcb de la lista ready a la lista susp_ready, segun el algoritmo de planificacion de largo plazo./
 * 
 * @param pcb: El pcb a mover
 * 
 */
int mover_a_suspready(t_pcb* pcb);

/**
 * @brief Mueve un pcb de la lista executing a la lista blocked_io
 * 
 * @return 0 si el pcb se mueve correctamente.
 * 
 * Esta función recibe un pid y un nombre de io. Mueve un pcb de la lista executing a la lista blocked_io
 * 
 */
int mover_executing_a_blockedio(u_int32_t pid, char* nombre_io, u_int32_t tiempo_io);


/**
 * @brief Mueve un pcb de la lista executing a la lista exit
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_executing_a_exit(u_int32_t pid);

/**
 * @brief Mueve un pcb de la lista executing a la lista blocked
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_executing_a_blocked(u_int32_t pid);

/**
 * @brief Mueve un pcb de la lista executing a la lista ready.
 * Se usa cuando una CPU se desconecta inesperadamente.
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_executing_a_ready(u_int32_t pid);

/**
 * @brief Mueve un pcb de la lista blocked a la lista ready
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_blocked_a_ready(u_int32_t pid);

/**
 * @brief Mueve un pcb de la lista blocked_io a la lista exit
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_blockedio_a_exit(u_int32_t pid);


/**
 * @brief Mueve un pcb de la lista blocked a la lista exit
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_blocked_a_exit(u_int32_t pid);

/**
 * @brief Mueve un pcb de la lista blocked a la lista susp_blocked
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_blocked_a_suspblocked(u_int32_t pid);

/**
 * @brief Mueve un pcb de la lista susp_blocked a la lista susp_ready
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_suspblocked_a_suspready(u_int32_t pid);

/**
 * @brief Mueve un pcb de la lista susp_ready a la lista ready
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_suspready_a_ready(u_int32_t pid);

/**
 * @brief Mueve un pcb de la lista ready a la lista executing
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_ready_a_executing(u_int32_t pid);

/**
 * @brief Mueve un pcb de la lista susp_blocked a la lista exit
 * 
 * @param pid: El pid del pcb a mover
 * 
 */
int mover_suspblocked_a_exit(u_int32_t pid);

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones auxiliares para listas y colas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/**
 * @brief Peek de la lista new
 * 
 * @return El pcb en la posicion 0 de la lista new
 */ 
t_pcb* peek_lista_new();

/**
 * @brief Peek de la lista ready
 * 
 * @return El pcb en la posicion 0 de la lista ready
 */
t_pcb* peek_lista_ready();


/**
 * @brief Peek de la lista susp_ready
 * 
 * @return El pcb en la posicion 0 de la lista susp_ready
 */
t_pcb* peek_lista_suspready();


/**
 * @brief Obtiene un pcb de la lista new
 * 
 * Esta función no recibe parámetros. Obtiene un pcb de la lista new
 * 
 */
t_pcb* obtener_pcb_de_lista_new();

/**
 * @brief Obtiene un pcb de la lista susp_ready
 * 
 * @return pcb de la lista susp_ready o NULL si esta vacia.
 */
t_pcb* obtener_pcb_de_lista_suspready();

/**
 * @brief Obtiene un pcb de la lista executing sin removerlo
 * 
 * @param pid: El pid del pcb a obtener
 * @return pcb de la lista executing o NULL si no se encuentra.
 */
t_pcb* obtener_pcb_de_lista_executing(u_int32_t pid);

/**
 * @brief Saca un t_blocked_io de la lista blocked_io
 * 
 * @param io_a_sacar: El t_blocked_io a sacar de la lista blocked_io
 * 
 */
int sacar_de_blockedio(t_blocked_io* io_a_sacar);


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones auxiliares de busqueda

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/**
 * @brief Compara el socket de un IO
 * 
 * Esta función compara el socket recibido en el segundo parametro con el socket de una instancia de IO
 * 
 */
bool comparar_socket_io(void* elemento, void* socket_a_comparar);

/**
 * @brief Compara el tamaño de dos PCBs.
 * 
 * @param elemento Un t_pcb
 * @param otro_elemento Otro t_pcb
 * @return true si el primer pcb es mas chico que el segundo.
*/
bool es_mas_chico(void* elemento, void* otro_elemento);

/**
 * @brief Recibe un socket de una instancia de IO, y busca el IO en la lista de IO
 * 
 * @param socket_io Socket buscado
 * @return IO encontrado
 */
t_io* buscar_io_por_socket(int socket_io);

t_io* buscar_io_por_socket_unsafe(int socket_io);

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

/**
 * @brief Itera la lista de exit y pide a memoria que se eliminen los procesos.
 * 
 */
void eliminar_procesos_en_exit();

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Actualizacion de PCB

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Actualiza el PC del pcb en la lista blocked
 * 
 * @param pid: El pid del pcb a actualizar
 * @param pc: El nuevo valor del PC
 * @return 0 si el pcb se actualiza correctamente
 */
int actualizar_pcb_en_blocked(u_int32_t pid, u_int32_t pc);

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones para planificacion mediano plazo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/**
 * @brief Llama al planificador de mediano plazo. El mismo inicia un timer para determinar si debe mover un pcb a susp blocked.
 * 
 * Esta función recibe un pcb y llama a la planificacion mediano plazo
 * 
 */
void* llamar_planificacion_mediano_plazo(void* pcb);



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones manejo de t_cpu_en_kernel

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


/**
 * @brief Busca una CPU por su id
 * 
 * @param id_cpu: El id de la CPU a buscar
 * @return La CPU encontrada o NULL si no existe
 */
t_cpu_en_kernel* buscar_cpu_por_id_unsafe(int id_cpu);



/**
 * @brief Obtiene una CPU libre
 * 
 * @return La CPU encontrada o NULL si no existe
 */
t_cpu_en_kernel* obtener_cpu_libre();

/**
 * @brief Busca una CPU por su socket
 * 
 * @param socket Socket de la CPU
 * @return El indice de la CPU encontrada o -1 si no existe
 */
int buscar_cpu_por_socket_unsafe(int socket);

/**
 * @brief Asigna un pcb a una cpu
 * 
 * @param pcb: El pcb a asignar
 * @return 0 si el pcb se asigna correctamente
 */
int asignar_pcb_a_cpu(t_pcb* pcb);

/**
 * @brief Obtiene la cpu que esta ejecutando un proceso
 * 
 * @param pid: El pid del proceso
 * @return La cpu que esta ejecutando el proceso o -1 si no existe
 */
t_cpu_en_kernel* obtener_cpu_por_pid(u_int32_t pid);

/**
 * @brief Libera una cpu, cambia el valor de esta_ocupada a false y el pid_actual a -1 para indicar que esta libre
 * 
 * @param cpu: La cpu a liberar
 * @return 0 si la cpu se libera correctamente
 */
int liberar_cpu(t_cpu_en_kernel* cpu);




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
 * @brief Recibe una estructura t_syscall, maneja el string con la instruccion contenido en dicha estructura e interpreta el tipo de syscall
 * 
 * @param syscall: La estructura t_syscall que contiene la instruccion a manejar
 * @return El resultado de la syscall
 */
int manejar_syscall(t_syscall* syscall);


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones que ejecutan la syscall propiamente

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



/**
 * @brief Inicializa un proceso. Esta syscall se llama cuando CPU ejecute la instruccion INIT_PROC
 * 
 * @param archivo_pseudocodigo: El archivo de pseudocódigo del proceso
 * @param tamanio_proceso: El tamaño del proceso
 */
int init_proc(char* archivo_pseudocodigo, u_int32_t tamanio_proceso);


/**
 * @brief Ejecuta la syscall de IO
 * 
 * @param nombre_io: El nombre del IO
 * @param tiempo_io: El tiempo de IO en milisegundos
 */
int io (char* nombre_io, u_int32_t tiempo_io, u_int32_t pid);

/**
 * @brief Ejecuta la syscall de exit
 * 
 * @param pid: El PID del proceso a finalizar
 * @return 0 si la syscall se ejecuta correctamente
 */
int exit_syscall(u_int32_t pid);



/**
 * @brief Ejecuta la syscall de dump_memory
 * 
 * @return 0 si la syscall se ejecuta correctamente
 */
int dump_memory(u_int32_t pid);



#endif
