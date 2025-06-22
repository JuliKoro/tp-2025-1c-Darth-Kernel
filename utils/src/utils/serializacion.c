#include "serializacion.h"


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de buffer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


t_buffer *buffer_create(uint32_t size) {
    t_buffer *buffer = malloc(sizeof(t_buffer));
    buffer->size = size;
    buffer->offset = 0;
    buffer->stream = malloc(size); //Recordar liberar
    return buffer;
}

void buffer_destroy(t_buffer *buffer) {
    if(buffer){
        free(buffer->stream);
        free(buffer);
    }
}

void buffer_add(t_buffer *buffer, void *data, uint32_t size) {
    if(buffer->offset + size > buffer->size){
        printf("Error: Buffer overflow\n");
        exit(1);
    }
    memcpy(buffer->stream + buffer->offset, data, size);
    buffer->offset += size;
}

void buffer_read(t_buffer *buffer, void *data, uint32_t size) {
    memcpy(data, buffer->stream + buffer->offset, size);
    buffer->offset += size;
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de buffer especificas

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



void buffer_add_uint32(t_buffer *buffer, uint32_t data) {
    buffer_add(buffer, &data, sizeof(uint32_t));
}

uint32_t buffer_read_uint32(t_buffer *buffer) {
    uint32_t data;
    buffer_read(buffer, &data, sizeof(uint32_t));
    return data;
}

void buffer_add_uint8(t_buffer *buffer, uint8_t data) {
    buffer_add(buffer, &data, sizeof(uint8_t));
}

uint8_t buffer_read_uint8(t_buffer *buffer) {   
    uint8_t data;
    buffer_read(buffer, &data, sizeof(uint8_t));
    return data;
}

void buffer_add_string(t_buffer *buffer, uint32_t length, char *string) {   
    buffer_add_uint32(buffer, length);
    buffer_add(buffer, string, length);
}

char *buffer_read_string(t_buffer *buffer, uint32_t *length) {
    *length = buffer_read_uint32(buffer);
    char *string = malloc(*length + 1); //Recordar liberar
    buffer_read(buffer, string, *length);
    string[*length] = '\0';
    return string;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de serializacion y deserializacion IO

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_buffer* serializar_solicitud_io(t_solicitud_io* solicitud) {
    t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));
    buffer_add_uint32(buffer, solicitud->pid);
    buffer_add_uint32(buffer, solicitud->tiempo);
    return buffer;
}

t_solicitud_io* deserializar_solicitud_io(t_buffer* buffer) {
    t_solicitud_io* solicitud = malloc(sizeof(t_solicitud_io));
    solicitud->pid = buffer_read_uint32(buffer);
    solicitud->tiempo = buffer_read_uint32(buffer);
    return solicitud;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de serializacion y deserializacion CPU

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_buffer* serializar_instruccion_cpu(t_instruccion_cpu* instruccion) {
    t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));
    buffer_add_uint32(buffer, instruccion->pid);
    buffer_add_uint32(buffer, instruccion->pc);
    return buffer;
}

t_instruccion_cpu* deserializar_instruccion_cpu(t_buffer* buffer) {
    t_instruccion_cpu* instruccion = malloc(sizeof(t_instruccion_cpu));
    instruccion->pid = buffer_read_uint32(buffer);
    instruccion->pc = buffer_read_uint32(buffer);
    return instruccion;
}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de empaquetado y desempaquetado

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_paquete* empaquetar_buffer(t_codigo_operacion codigo_operacion, t_buffer* buffer) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = codigo_operacion;
    paquete->buffer = buffer;
    return paquete;
}




/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de armado de streams, para enviar

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void* stream_para_enviar(t_paquete* paquete){
    void* stream = malloc(paquete->buffer->size + sizeof(t_codigo_operacion) + sizeof(uint32_t));
    int offset = 0;
    memcpy(stream + offset, &(paquete->codigo_operacion), sizeof(t_codigo_operacion));
    offset += sizeof(t_codigo_operacion);

    memcpy(stream + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, paquete->buffer->stream, paquete->buffer->size);
    return stream; //Recordar liberar en donde se llame
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de enviado y recepcion de paquetes

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void enviar_paquete(int socket, t_paquete* paquete){
    void* stream = stream_para_enviar(paquete);
    send(socket, stream, paquete->buffer->size + sizeof(t_codigo_operacion) + sizeof(uint32_t), 0);
    liberar_paquete(paquete);
    free(stream);
}

t_paquete* recibir_paquete(int socket){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    //Recibir codigo de operacion
    recv(socket, &(paquete->codigo_operacion), sizeof(t_codigo_operacion), 0);
    
    //Recibir tamaño del buffer
    recv(socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    paquete->buffer->offset = 0;

    //Recibir buffer
    recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);

    return paquete; //Recordar liberar en donde se llame
}

void liberar_paquete(t_paquete* paquete){
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}