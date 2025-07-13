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

t_buffer* serializar_proceso_cpu(t_proceso_cpu* proceso) {
    t_buffer* buffer = buffer_create(2 * sizeof(uint32_t));
    buffer_add_uint32(buffer, proceso->pid);
    buffer_add_uint32(buffer, proceso->pc);
    return buffer;
}

t_proceso_cpu* deserializar_proceso_cpu(t_buffer* buffer) {
    t_proceso_cpu* proceso = malloc(sizeof(t_proceso_cpu));
    proceso->pid = buffer_read_uint32(buffer);
    proceso->pc = buffer_read_uint32(buffer);
    return proceso;
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

int enviar_paquete(int socket, t_paquete* paquete){
    //Arma el stream a partir del paquete, que contiene un codigo de operacion y un buffer
    void* stream = stream_para_enviar(paquete);
    //Envia el stream al socket recibido por parametro
    if(send(socket, stream, paquete->buffer->size + sizeof(t_codigo_operacion) + sizeof(uint32_t), 0) == -1){
        liberar_paquete(paquete); //Libera el paquete recibido por parametro porque fallo el envio
        free(stream); //Libera el stream creado despues de enviarlo porque fallo el envio
        return -1;
    };
    liberar_paquete(paquete); //Libera el paquete recibido por parametro despues de enviarlo
    free(stream); //Libera el stream creado despues de enviarlo
    return 0;
}

t_paquete* recibir_paquete(int socket){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    if(paquete == NULL){
        return NULL;
    }
    paquete->buffer = malloc(sizeof(t_buffer));
    if(paquete->buffer == NULL){
        free(paquete);
        return NULL;
    }

    //Recibir codigo de operacion
    if (recv(socket, &(paquete->codigo_operacion), sizeof(t_codigo_operacion), MSG_WAITALL) <= 0) {
        free(paquete->buffer);
        free(paquete);
        return NULL;
    }
    
    //Recibir tamaño del buffer
     if (recv(socket, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL) <= 0) {
        free(paquete->buffer);
        free(paquete);
        return NULL;
    }
    
    if (paquete->buffer->size > 0) {
        paquete->buffer->stream = malloc(paquete->buffer->size);
        if (paquete->buffer->stream == NULL) {
             free(paquete->buffer);
             free(paquete);
             return NULL;
        }
        if (recv(socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL) <= 0) {
            free(paquete->buffer->stream);
            free(paquete->buffer);
            free(paquete);
            return NULL;
        }
    } else {
        paquete->buffer->stream = NULL;
    }
    paquete->buffer->offset = 0;

    return paquete; //Recordar liberar en donde se llame
}

void liberar_paquete(t_paquete* paquete){
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de serializacion y deserializacion PCB

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_buffer* serializar_pcb(t_pcb* pcb){
    uint32_t tamanio_string = strlen(pcb->archivo_pseudocodigo);
    uint32_t tamanio_total = 3 * sizeof(uint32_t) + sizeof(uint32_t) + tamanio_string;
    t_buffer* buffer = buffer_create(tamanio_total);
    buffer_add_uint32(buffer, pcb->pid);
    buffer_add_uint32(buffer, pcb->pc);
    buffer_add_uint32(buffer, pcb->tamanio_proceso);
    buffer_add_string(buffer, tamanio_string, pcb->archivo_pseudocodigo);
    return buffer;
}

t_pcb* deserializar_pcb(t_buffer* buffer){
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->pid = buffer_read_uint32(buffer);
    pcb->pc = buffer_read_uint32(buffer);
    pcb->tamanio_proceso = buffer_read_uint32(buffer);

    uint32_t length;
    pcb->archivo_pseudocodigo = buffer_read_string(buffer, &length);
    return pcb;
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                        Funciones de serializacion y deserializacion syscalls

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_buffer* serializar_syscall(t_syscall* syscall){
    t_buffer* buffer = buffer_create(sizeof(t_syscall));
    buffer_add_uint32(buffer, syscall->pid);
    buffer_add_string(buffer, strlen(syscall->syscall), syscall->syscall);
    return buffer;
}

t_syscall* deserializar_syscall(t_buffer* buffer){
    t_syscall* syscall = malloc(sizeof(t_syscall));
    uint32_t length;
    syscall->syscall = buffer_read_string(buffer, &length);
    syscall->pid = buffer_read_uint32(buffer);
    return syscall;
}