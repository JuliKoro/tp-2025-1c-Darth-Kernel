#ifndef SERIALIZACION_H
#define SERIALIZACION_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#include "pcb.h"


//Serializar consiste en armar un stream a partir de una estructura. Se guardan todos los contenidos de la estructura en el stream. o buffer
//Empaquetarlo consiste en armar una estructura t_paquete que contiene un codigo de operacion y un buffer.

//Deserializar consiste en leer el stream y reconstruir la estructura original.
//Desempaquetar consiste en recibir un t_paquete, extraer el codigo de operacion. Y segun el mismo, 
//deserializar el buffer en la estructura correspondiente.

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Estructuras usadas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

//Codigos de operacion

/**
 * @enum t_codigo_operacion
 * @brief Enum que representa los códigos de operación para los paquetes
 * 
 * @param PAQUETE_SOLICITUD_IO: Código de operación para solicitud de IO
 * @param PAQUETE_PROCESO_CPU: Código de operación para instrucción del CPU (Kernel-CPU y CPU-Memoria)
 * @param PAQUETE_PCB: Código de operación para PCB
 * @param PAQUETE_SYSCALL: Código de operación para syscall
 * @param PAQUETE_INTERRUPCION: Código de operación para interrupcion de un proceso
 * @param PAQUETE_INFO_TP: Código de operación para handshake Memoria-CPU (Info Tabla de Paginas)
 * @param PAQUETE_SOLICITUD_MARCO: Código de operación cuando CPU solicita el Marco de una pagina a Memoria (Para la MMU)
 * @param PAQUETE_MARCO: Código de operación, rta a PAQUETE_SOLICITUD_MARCO por parte de Memoria a CPU
 * @param PAQUETE_READ: Código de operación cuando CPU quiere leer el valor en una determinada direccion en Memoria
 * @param PAQUETE_WRITE: Código de operación cuando CPU quiere escribir un valor en una determinada direccion en Memoria
 */
typedef enum {
    PAQUETE_SOLICITUD_IO=1,
    PAQUETE_PROCESO_CPU=2, //ok
    PAQUETE_PCB=3,
    PAQUETE_SYSCALL=4,
    PAQUETE_INTERRUPCION=5,
    PAQUETE_INFO_TP=6, // hacer unicamente en el hs
    PAQUETE_ELIMINAR_PROCESO=7,
    PAQUETE_SUSPENDER_PROCESO=8,
    PAQUETE_CARGAR_PROCESO=9,
    PAQUETE_DUMP_MEMORY=10,
    PAQUETE_SOLICITUD_MARCO=11, //
    PAQUETE_MARCO=12,
    PAQUETE_READ=13, //ok
    PAQUETE_WRITE=14, //ok
} t_codigo_operacion;

//Estructura de mensaje para modulo IO

/**
 * @struct t_solicitud_io
 * @brief Estructura que representa una solicitud de IO (PID y tiempo)
 * 
 * @param pid: Identificador del proceso asociado a la solicitud de IO
 * @param tiempo: Tiempo de la solicitud de IO
 */
typedef struct {
    uint32_t pid;
    uint32_t tiempo;
} t_solicitud_io;

//Estructura de proceso para modulo CPU

/**
 * @struct t_proceso_cpu
 * @brief Estructura que representa un proceso del Kernel (PID y PC)
 * 
 * @param pid: Identificador del proceso asociado a la instrucción
 * @param pc: Contador de programa que indica la dirección de la instrucción
 */
typedef struct {
    uint32_t pid; // Identificador del proceso asociado a la instrucción
    uint32_t pc;  // Contador de programa que indica la dirección de la instrucción
} t_proceso_cpu;

// Estructuras usadas entre CPU y MEMORIA

/**
 * @struct t_tabla_pag
 * @brief Estructura que contien info sobre la Tabla de Paginas en Memoria
 * 
 * @param tamanio_pagina Tamaño de cada pagina en memoria (en bytes)
 * @param cant_entradas_tabla Cantidad de entradas de cada tabla de páginas
 * @param cant_niveles Cantidad de niveles de tablas de páginas
 * 
 * @note Utilizada para Handshake entre CPU y Memoria.
 *       COD_OP (paquete): PAQUETE_INFO_TP
 */
typedef struct {
    uint32_t tamanio_pagina; // Tamaño de cada pagina en memoria (en bytes)
    uint32_t cant_entradas_tabla; // Cantidad de entradas de cada tabla de páginas
    uint32_t cant_niveles; // Cantidad de niveles de tablas de páginas
} t_tabla_pag;

/**
 * @brief Estrucutra utilizada para solicitar a Memoria cargar/leer una pagina con su contenido
 * 
 * @param pid ID del proceso que necesita cargar/leer la pagina
 * @param direccion_fisica Direccion fisica de la pagina que se desea cargar/leer
 * @param tamanio Tamanio de lo que se pretende leer (SOLO CUANDO SE USA PAQUETE_READ)
 * 
 * @note Se peude usar tanto para solicitar la carga de pagina a la cache (PAQUETE_SOLICITUD_PAG) como para la instruccion de READ (PAQUETE_READ). Lo usan CPU, Memoria
 */
typedef struct {
    uint32_t pid; // ID del proceso que necesita cargar la pagina 
    uint32_t direccion_fisica; // Direccion fisica de la pagina que se desea cargar/leer
    uint32_t tamanio; // Tamanio de lo que se pretende leer (SOLO CUANDO SE USA PAQUETE_READ)
} t_lectura_memoria;

/**
 * @brief Estrucutra utilizada para solicitar a Memoria actulizar/escribir una pagina con su contenido
 * 
 * @param pid ID del proceso que necesita actulizar/escribir la pagina
 * @param direccion_fisica Direccion fisica de la pagina que se desea actulizar/escribir
 * @param tamanio Tamanio de lo que se pretende actulizar/escribir
 * @param dato Dato que se quiere actulizar/escribir en Memoria
 * 
 * @note Se peude usar tanto para solicitar la actualizacion de pagina de la cache (PAQUETE_SOLICITUD_PAG) como para la instruccion de WRITE (PAQUETE_WRITE). Lo usan CPU, Memoria
 */
typedef struct {
    uint32_t pid;
    uint32_t direccion_fisica;
    uint32_t tamanio; // REVISAR SI ES NECESARIO
    void* dato;
} t_escritura_memoria;

/**
 * @brief Estrucutra utilizada para solicitar a Memoria un numero de marco (frame)
 * 
 * @param pid ID del proceso
 * @param entradas_niveles Arreglo de enteros, con las disitntas ntradas a los niveles de TP
 * @param num_pag Numero de Pagina del que se requiere el numero de marco
 */
typedef struct {
    uint32_t pid; //ID del proceso
    uint32_t* entradas_niveles; //Arreglo de enteros, con las disitntas ntradas a los niveles de TP
    uint32_t num_pag; //Numero de Pagina del que se requiere el numero de marco
} t_entradas_tabla;

/**
 * @brief Codigos de Operacion para la respuesta del Marco por parte de Memoria a CPU
 * 
 * @param MARCO_OBTENIDO Marco encontrado
 * @param ERROR_MEMORIA Error ocurriod en memoria, no se encontro el marco
 */
typedef enum {
    MARCO_OBTENIDO=1,
    ERROR_MEMORIA=2
} op_code;

/**
 * @brief Estructura para serilizar y deserializar el numero de marco, con un codigo de operacion (OK/ERROR)
 * 
 * @param marco Numero de Marco hallado
 * @param codigo_operacion Indica si se encontro correctamente o si ocurrio un error
 */
typedef struct {
    uint32_t marco; // Numero de marco encontrado
    op_code codigo_operacion; // Codigo de operacion que dice si se pudo encontrar el marco o hubo un error.
} t_marco;

//Estructura de buffer para serializacion y deserializacion

/**
 * @struct t_buffer
 * @brief Estructura que representa un buffer para serialización y deserialización. 
 * Contiene un stream(el payload), el tamaño del payload y un offset que representa la posicion actual en el stream.
 * 
 * @param size: Tamaño del payload
 * @param offset: Desplazamiento dentro del payload
 * @param stream: Payload
 */
typedef struct {
    uint32_t size; // Tamaño del payload
    uint32_t offset; // Desplazamiento dentro del payload
    void* stream; // Payload
} t_buffer;

//Estructura de paquete para enviar y recibir paquetes

/**
 * @struct t_paquete
 * @brief Estructura que representa un paquete para enviar y recibir paquetes.
 * Contiene un codigo de operacion (que sirve para saber como deserializar el buffer) y un buffer.
 * 
 * @param codigo_operacion: Código de operación para el paquete
 * @param buffer: Buffer que contiene el payload
 */
typedef struct {
    t_codigo_operacion codigo_operacion;
    t_buffer* buffer;
} t_paquete;

//Estructura de syscall

/**
 * @struct t_syscall
 * @brief Estructura que representa una syscall. Tiene el string con la instruccion syscall y el pid del proceso que la llamo.
 * 
 * @param syscall: String con la instruccion syscall
 * @param pid: Identificador del proceso que la llamo
 */
typedef struct {
    char* syscall;
    uint32_t pid;
} t_syscall;

//Estructura de interrupcion
typedef enum {
    INTERRUPCION_BLOQUEO=1,
    INTERRUPCION_FIN_EJECUCION=2
} t_motivo_interrupcion;

/**
 * @struct t_interrupcion
 * @brief Estructura que representa una interrupcion. Tiene el pid del proceso al que se quiere interrumpir y el motivo de la interrupcion.
 * 
 * @param pid: Identificador del proceso al que se quiere interrumpir
 * @param pc: PC del proceso al que se quiere interrumpir
 * @param rafaga_cpu: Cantidad de ciclos de maquina ejecutados del proceso que se desaloja
 * @param motivo: Motivo de la interrupcion
 */
typedef struct {
    uint32_t pid;
    uint32_t pc;
    t_motivo_interrupcion motivo;
} t_interrupcion;

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de buffer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// Crea un buffer vacío de tamaño size y offset 0
/**
 * @brief Crea un buffer vacío de tamaño size y offset 0
 * 
 * @param size Tamaño del buffer
 * @return t_buffer* Puntero al buffer creado
 */
t_buffer *buffer_create(uint32_t size);

// Libera la memoria asociada al buffer
/**
 * @brief Libera la memoria asociada al buffer
 * 
 * @param buffer Puntero al buffer a liberar
 */
void buffer_destroy(t_buffer *buffer);

// Agrega un stream al buffer en la posición actual y avanza el offset
/**
 * @brief Agrega un stream al buffer en la posición actual y avanza el offset
 * 
 * @param buffer Puntero al buffer
 * @param data Puntero al stream a agregar
 * @param size Tamaño del stream a agregar
 */
void buffer_add(t_buffer *buffer, void *data, uint32_t size);

// Guarda size bytes del principio del buffer en la dirección data y avanza el offset
/**
 * @brief Guarda size bytes del principio del buffer en la dirección data y avanza el offset
 * 
 * @param buffer Puntero al buffer
 * @param data Puntero a la dirección donde se guardarán los datos
 * @param size Tamaño de los datos a guardar
 */
void buffer_read(t_buffer *buffer, void *data, uint32_t size);


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de buffer especificas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// Agrega un uint32_t al buffer
void buffer_add_uint32(t_buffer *buffer, uint32_t data);

// Lee un uint32_t del buffer y avanza el offset
uint32_t buffer_read_uint32(t_buffer *buffer);

// Agrega un uint8_t al buffer
void buffer_add_uint8(t_buffer *buffer, uint8_t data);

// Lee un uint8_t del buffer y avanza el offset
uint8_t buffer_read_uint8(t_buffer *buffer);

// Agrega string al buffer con un uint32_t adelante indicando su longitud
void buffer_add_string(t_buffer *buffer, uint32_t length, char *string);

// Lee un string y su longitud del buffer y avanza el offset
char *buffer_read_string(t_buffer *buffer, uint32_t *length);


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de serializacion y deserializacion IO

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

//Recibe una solicitud de IO y la serializa
/**
 * @brief Recibe una solicitud de IO y la serializa
 * 
 * @param solicitud Puntero a la estructura de solicitud de IO
 * @return t_buffer* Puntero al buffer que contiene la solicitud de IO
 */
t_buffer* serializar_solicitud_io(t_solicitud_io* solicitud);

//Recibe un buffer y lo deserializa en una solicitud de IO 
/**
 * @brief Recibe un buffer y lo deserializa en una solicitud de IO
 * 
 * @param buffer Puntero al buffer que contiene la solicitud de IO
 * @return t_solicitud_io* Puntero a la estructura de solicitud de IO
 */
t_solicitud_io* deserializar_solicitud_io(t_buffer* buffer);

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de serializacion y deserializacion CPU

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Serializa una proceso del Kernel (PID+PC) en un buffer.
 * 
 * Se utiliza para enviar el PID y PC de un proceso a un CPU.
 * 
 * @param proceso Puntero a la estructura de proceso del Kernel
 *                    que se desea serializar.
 * @return t_buffer* Puntero al buffer que contiene el proceso
 *                   serializado. El buffer debe ser liberado.
 */
t_buffer* serializar_proceso_cpu(t_proceso_cpu* proceso);

/**
 * @brief Deserializa un proceso del Kernel desde un buffer
 * 
 * Se utiliza para recibir PID+PC a través de
 * la red o para leerlas desde un formato binario.
 * 
 * @param buffer Puntero al buffer que contiene el proceso
 *               serializada.
 * @return t_proceso* Puntero a la estructura de proceso
 *                             del Kernel deserializada. Se debe liberar.
 */
t_proceso_cpu* deserializar_proceso_cpu(t_buffer* buffer);

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de empaquetado y desempaquetado

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_paquete* empaquetar_buffer(t_codigo_operacion codigo_operacion, t_buffer* buffer);



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de armado de streams

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void* stream_para_enviar(t_paquete* paquete);


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de enviado y recepcion de paquetes

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Recibe un paquete, arma un stream con los contenidos del paquete y lo envia a un socket. Esta funcion ya libera el paquete despues de enviarlo.
 * 
 * @param socket Descriptor del socket
 * @param paquete Puntero al paquete a enviar
 * @return int 0 si el envío fue exitoso, -1 si hubo un error
 */
int enviar_paquete(int socket, t_paquete* paquete);

/**
 * @brief Recibe un stream que fue armado a partir de un paquete, lo desarma, pone los contenidos en un paquete y lo devuelve
 * 
 * @param socket Descriptor del socket
 * @return t_paquete* Puntero al paquete recibido
 */
t_paquete* recibir_paquete(int socket);

/**
 * @brief Libera la memoria de un paquete
 * 
 * @param paquete Puntero al paquete a liberar
 */
void liberar_paquete(t_paquete* paquete);


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de serializacion y deserializacion PCB

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

//Serializa las partes necesarias de un pcb para enviarlas a memoria
/**
 * @brief Serializa las partes necesarias de un pcb para enviarlas a memoria o CPU
 * 
 * @param pcb Puntero a la estructura de pcb
 * @return t_buffer* Puntero al buffer que contiene la serializacion del pcb
 */
t_buffer* serializar_pcb(t_pcb* pcb);

//Deserializa un buffer en una estructura de pcb
/**
 * @brief Deserializa un buffer en una estructura de pcb
 * 
 * @param buffer Puntero al buffer que contiene la serializacion del pcb
 * @return t_pcb* Puntero a la estructura de pcb
 */
t_pcb* deserializar_pcb(t_buffer* buffer);

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de serializacion y deserializacion syscalls

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

//Serializa una syscall
/**
 * @brief Serializa una syscall
 * 
 * @param syscall Puntero a la estructura de syscall. Deberia ser un string, hasta el primer espacio es la instruccion, 
 * y el resto son parametros
 * @return t_buffer* Puntero al buffer que contiene la serializacion de la syscall
 */
t_buffer* serializar_syscall(t_syscall* syscall);

//Deserializa un buffer en una syscall
/**
 * @brief Deserializa un buffer en una syscall
 * 
 * @param buffer Puntero al buffer que contiene la serializacion de la syscall
 * @return t_syscall* Puntero a la estructura de syscall
 */
t_syscall* deserializar_syscall(t_buffer* buffer);

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                        Funciones de serializacion y deserializacion interrupciones

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Serializa una interrupcion
 * 
 * @param pid ID del proceso al que se quiere interrumpir
 * @return t_buffer* Puntero al buffer que contiene la serializacion de la interrupcion
 */
t_buffer* serializar_interrupcion(t_interrupcion* interrupcion);

/**
 * @brief Deserializa un buffer en una interrupcion
 * 
 * @param buffer Puntero al buffer que contiene la serializacion de la interrupcion
 * @return t_interrupcion* Puntero a la estructura de interrupcion
 */
t_interrupcion* deserializar_interrupcion(t_buffer* buffer);

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                    Funciones de serializacion y deserializacion CPU y Memoria

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/**
 * @brief Serializa una info de Tabla de Paginas
 * 
 * @param info_tabla_pag Estructura con informacion de la Tabla de Pagians en Memoria
 * @return t_buffer* Puntero al buffer que contiene la serializacion de info_tabla_pag
 * @note Utilizado para HANDSHAKE entre CPU y MEMORIA
 */
t_buffer* serializar_info_tabla_pag(t_tabla_pag* info_tabla_pag);

/**
 * @brief Deserializa un buffer en una interrupcion
 * 
 * @param buffer Puntero al buffer que contiene la serializacion de info_tabla_pag
 * @return t_tabla_pag* Puntero a la estructura de info_tabla_pag
 * @note Utilizado para HANDSHAKE entre CPU y MEMORIA
 */
t_tabla_pag* deserializar_info_tabla_pag(t_buffer* buffer);

/**
 * @brief Serializa una solicitud para leer o cargar una Pagina
 * 
 * @param solicitud_lectura Estructura con pid y DF a leer/cargar
 * @return t_buffer* Puntero al buffer que contiene la serializacion de t_lectura_memoria
 * @note Utilizado para cargar una pagina de Memoria a la Cache (CPU) / Leer directo de Memoria (READ)
 *       COD_OP: PAQUETE_SOLICITUD_PAG / PAQUETE_READ
 */
t_buffer* serializar_lectura_memoria(t_lectura_memoria* solicitud_lectura);

/**
 * @brief Deserializa un buffer de una solicitud para leer o cargar una Pagina
 * 
 * @param buffer Puntero al buffer que contiene la serializacion de t_lectura_memoria
 * @return t_lectura_memoria* Puntero a la estructura de t_lectura_memoria
 * @note Utilizado para cargar una pagina de Memoria a la Cache (CPU) / Leer directo de Memoria (READ)
 *       COD_OP: PAQUETE_SOLICITUD_PAG / PAQUETE_READ
 */
t_lectura_memoria* deserializar_lectura_memoria(t_buffer* buffer);

/**
 * @brief Serializa una paquete de escritura para WRITE o Actualizar Pagina
 * 
 * @param escritura_memoria Estructura con pid, DF, tamanio y datos a escribir en Memoria
 * @return t_buffer* Puntero al buffer que contiene la serializacion de t_escritura_memoria
 * @note Utilizado para actualizar una pagina de la Cache (CPU) en Memoria a la Cache / Escribir directo en Memoria (WRITE)
 *       COD_OP: PAQUETE_WRITE
 */
t_buffer* serializar_escritura_memoria(t_escritura_memoria* escritura_memoria);

/**
 * @brief Deserializa un buffer para la escritura de datos en Memoria
 * 
 * @param buffer Puntero al buffer que contiene la serializacion de t_escritura_memoria
 * @return t_escritura_memoria* Puntero a la estructura de t_escritura_memoria
 * @note Utilizado para escribir/actualizar una pagina en Memoria (datos provenientes de CPU)
 *       COD_OP: PAQUETE_WRITE
 */
t_escritura_memoria* deserializar_escritura_memoria(t_buffer* buffer);

/**
 * @brief Serializa una solicitud de marco en un buffer.
 *
 * @param entradas_tabla Puntero a la estructura de entradas de tabla que se desea serializar.
 * @param cant_niveles Cantidad de niveles en la tabla de páginas (cargado de las configs).
 * @return t_buffer* Puntero al buffer que contiene la solicitud serializada.
 */
t_buffer* serializar_solicitud_marco(t_entradas_tabla* entrada_tabla, uint32_t cant_niveles);

/**
 * @brief Deserializa una solicitud de marco desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la solicitud serializada.
 * @param cant_niveles Cantidad de niveles en la tabla de páginas (cargado de las configs).
 * @return t_entradas_tabla* Puntero a la estructura de entradas de tabla deserializada.
 */
t_entradas_tabla* deserializar_solicitud_marco(t_buffer* buffer, uint32_t cant_niveles);

void liberar_entradas_tabla(t_entradas_tabla* entradas_tabla);

#endif


