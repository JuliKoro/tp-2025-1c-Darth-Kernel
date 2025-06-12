#ifndef SERIALIZACION_H
#define SERIALIZACION_H

#include <stdint.h>

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Estructuras usadas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

//Estructura de mensaje para modulo IO
typedef enum {
    PAQUETE_SOLICITUD_IO=1
} t_codigo_operacion;

typedef struct {
    uint32_t pid;
    uint32_t tiempo;
} t_solicitud_io;


//Estructura de buffer para serializacion y deserializacion

typedef struct {
    uint32_t size; // Tamaño del payload
    uint32_t offset; // Desplazamiento dentro del payload
    void* stream; // Payload
} t_buffer;


typedef struct {
    t_codigo_operacion codigo_operacion;
    t_buffer* buffer;
} t_paquete;



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de buffer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

// Crea un buffer vacío de tamaño size y offset 0
t_buffer *buffer_create(uint32_t size);

// Libera la memoria asociada al buffer
void buffer_destroy(t_buffer *buffer);

// Agrega un stream al buffer en la posición actual y avanza el offset
void buffer_add(t_buffer *buffer, void *data, uint32_t size);

// Guarda size bytes del principio del buffer en la dirección data y avanza el offset
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

t_buffer serializar_solicitud_io(t_solicitud_io* solicitud);

t_solicitud_io* deserializar_solicitud_io(t_buffer* buffer);

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

void enviar_paquete(int socket, t_paquete* paquete);

t_paquete* recibir_paquete(int socket);

void liberar_paquete(t_paquete* paquete);

#endif


