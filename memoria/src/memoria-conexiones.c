#include "memoria-conexiones.h"
#include "memoria-procesos.h"



int iniciar_servidor_memoria(){

    //Creo el socket del servidor
    int memoria_server_fd = iniciar_servidor(int_a_string(memoria_configs.puertoescucha));

    return memoria_server_fd;
}

void* manejar_conexion_cpu(void* socket_cliente){
    int socket_fd = (intptr_t)socket_cliente;

        //Recibir pid, tamanio, pc, archivo pseudocodigo
        t_paquete* paquete = recibir_paquete(socket_fd);

        //Evaluo paquete
        if(paquete == NULL){
            log_info(logger_memoria, "Cliente desconectado (socket %d)", socket_fd);
            close(socket_fd);
            return NULL;
        }
    
        if(paquete->codigo_operacion == PAQUETE_PROCESO_CPU){
            t_proceso_cpu* proceso_cpu = deserializar_proceso_cpu(paquete->buffer);
            char* instruccion = obtener_instruccion(proceso_cpu->pid, proceso_cpu->pc);
            if(instruccion == NULL){
                log_error(logger_memoria, "[PROCESO CPU] No se encontró el proceso con PID %d o PC %d fuera de rango (total instrucciones: %d).",
                    proceso_cpu->pid, proceso_cpu->pc);
            enviar_bool(socket_fd, false);
            } else {
                enviar_bool(socket_fd, true);
                enviar_mensaje(instruccion, socket_fd);
            }
        }
      //  if(paquete->codigo_operacion == PAQUETE_SOLICITUD_MARCO){
        //    t_entrada_tabla* entrada_tabla = deserializar_solicitud_marco(paquete->buffer);
          //  if()    ;
//            pid, entr nivel, num pag            
        if(paquete->codigo_operacion == PAQUETE_READ){
            t_lectura_memoria* datos_lectura_memoria = deserializar_lectura_memoria(paquete->buffer);
            void* memoria_leida = leer_memoria(datos_lectura_memoria->pid, datos_lectura_memoria->direccion_fisica, datos_lectura_memoria->tamanio);
            if(memoria_leida == NULL){
                log_error(logger_memoria, "[LECTURA MEMORIA] No se pudo leer la memoria del proceso %d y direccion fisica %d",
                    datos_lectura_memoria->pid, datos_lectura_memoria->direccion_fisica);
                enviar_bool(socket_fd, false);
            } else {
                enviar_bool(socket_fd, true);
                enviar_mensaje(memoria_leida, socket_fd); 
            }
        }
//int pid, int direccion_fisica, int tam, void* valor
        if(paquete->codigo_operacion == PAQUETE_WRITE){
            t_escritura_memoria* datos_escritura_memoria = deserializar_lectura_memoria(paquete->buffer);
            if(escribir_memoria(datos_escritura_memoria->pid, datos_escritura_memoria->direccion_fisica, 
                datos_escritura_memoria->tamanio, datos_escritura_memoria->valor) == false){
                log_error(logger_memoria, "[ESCRITURA MEMORIA] No se pudo escribir la memoria del proceso %d y direccion fisica %d",
                    datos_escritura_memoria->pid, datos_escritura_memoria->direccion_fisica);
                enviar_bool(socket_fd, false);
            } else {
                enviar_bool(socket_fd, true); 
            }
        }
        //Enviar respuesta al kernel     
        liberar_paquete(paquete);
        close(socket_fd);
    }

void* manejar_conexion_kernel(void* socket_cliente){
 int socket_fd = (intptr_t)socket_cliente;

        //Recibir pid, tamanio, pc, archivo pseudocodigo
        t_paquete* paquete = recibir_paquete(socket_fd);

        //Evaluo paquete
        if(paquete == NULL){
            log_info(logger_memoria, "Cliente desconectado (socket %d)", socket_fd);
            close(socket_fd);
            return NULL;
        }
        if(paquete->codigo_operacion == PAQUETE_CARGAR_PROCESO){
            t_pcb* pcb = deserializar_pcb(paquete->buffer);
            //Ver si puedo cargar instrucciones en memoria
            if(cargar_proceso(pcb->pid, pcb->archivo_pseudocodigo) == -1){
            log_error(logger_memoria, "[CARGAR PROCESO] Error al cargar proceso. PID: %d", pcb->pid);
            enviar_bool(socket_fd, false);
            } else {
                enviar_bool(socket_fd, true);
            }

        }
        if(paquete->codigo_operacion == PAQUETE_SUSPENDER_PROCESO){
            t_pcb* pcb = deserializar_pcb(paquete->buffer);
            //ver si puedo suspender proceso de memoria
            if(suspender_proceso(pcb->pid) == -1){
                log_error(logger_memoria, "[SUSPENDER PROCESO] Error al suspender proceso. PID: %d", pcb->pid);
            enviar_bool(socket_fd, false);
            } else {
                enviar_bool(socket_fd, true);
            }
        }
        if (paquete->codigo_operacion == PAQUETE_ELIMINAR_PROCESO){
            t_pcb* pcb = deserializar_pcb(paquete->buffer);
            //ver si puedo eliminar proceso de memoria
            if(finalizar_proceso(pcb->pid) == -1){
                log_error(logger_memoria, "[ELIMINAR PROCESO] Error al finalizar proceso. PID: %d", pcb->pid);
            enviar_bool(socket_fd, false);
            } else {
                enviar_bool(socket_fd, true);
            }
            //Aca tenes que devolverme un bool
        }
         if(paquete->codigo_operacion == PAQUETE_DUMP_MEMORY){
            t_pcb* pcb = deserializar_pcb(paquete->buffer);
            //ver si puedo suspender proceso de memoria
            if(realizar_memory_dump(pcb->pid) == -1){
                log_error(logger_memoria, "[MEMORIA DUMP PROCESO] Error al dumpear proceso. PID: %d", pcb->pid);
            enviar_bool(socket_fd, false);
            } else {
                enviar_bool(socket_fd, true);
            }
        }

}

 void* escuchar_peticiones(void* socket_memoria){
    int socket_fd = (intptr_t) socket_memoria;
    //Aca escucho peticiones en el socket, pueden ser de kernel o de cpu.
    while(true){
        int cliente_fd = esperar_cliente(socket_fd); //Se conecta un cliente
        
        int id_cpu = -1;
        recibir_handshake_cpu(cliente_fd, &id_cpu);
        if(id_cpu != -1){
            //Es una cpu
            t_tabla_pag* info_tabla_pag = malloc(sizeof(t_tabla_pag));
            info_tabla_pag->tamanio_pagina = memoria_configs.tampagina;
            info_tabla_pag->cant_entradas_tabla = memoria_configs.entradasportabla;
            info_tabla_pag->cant_niveles = memoria_configs.cantidadniveles;
            t_paquete* paquete = empaquetar_buffer(PAQUETE_INFO_TP, serializar_info_tabla_pag(info_tabla_pag));
            enviar_paquete(cliente_fd, paquete);
            free(info_tabla_pag);
            //Creo un hilo para atender la peticion
            pthread_t hilo_cliente;
            pthread_create(&hilo_cliente, NULL, manejar_conexion_cpu, (void*)(intptr_t) cliente_fd);
            pthread_detach(hilo_cliente);
          } else {
            //Es kernel
            pthread_t hilo_cliente;
            pthread_create(&hilo_cliente, NULL, manejar_conexion_kernel, (void*)(intptr_t) cliente_fd);
            pthread_detach(hilo_cliente);
          }

        
    }
}



void* recibir_peticiones_cpu(void* socket_cliente){

}







/*void* recibir_peticiones_cpu(void* socket_memoria) {
    int socket_fd = *(int*)socket_memoria;

    while (1) {
        int cliente_fd = esperar_cliente(socket_fd);

        pthread_t hilo_cpu;
        int* socket_cliente = malloc(sizeof(int));
        *socket_cliente = cliente_fd;

        pthread_create(&hilo_cpu, NULL, atender_peticion_cpu, (void*)socket_cliente);
        pthread_detach(hilo_cpu);
    }

    return NULL;
}
void* atender_peticion_cpu(void* socket_cliente) {
    int socket_fd = *(int*)socket_cliente;
    free(socket_cliente);

    id_modulo_t modulo;
    if (recibir_handshake(socket_fd, &modulo) == -1) {
        log_error(logger_memoria, "[HANDSHAKE] Error en conexión con CPU");
        close(socket_fd);
        return NULL;
    }

    log_info(logger_memoria, "[HANDSHAKE] CPU conectada correctamente - FD: %d", socket_fd);

    while (1) {
        op_code codigo_operacion;

        if (recv(socket_fd, &codigo_operacion, sizeof(op_code), 0) <= 0) {
            log_info(logger_memoria, "[CPU] Se cerró la conexión (FD: %d)", socket_fd);
            close(socket_fd);
            break;
        }

        log_debug(logger_memoria, "[CPU] Operación recibida: %d", codigo_operacion);

                IMPLEMENTAR FUNCIONES -- CHEQUEAR ANTES

        switch (codigo_operacion) {
            case SOLICITUD_INSTRUCCION:
                manejar_solicitud_instruccion(socket_fd);
                break;
            case LECTURA_MEMORIA:
                manejar_lectura_memoria(socket_fd);
                break;
            case ESCRITURA_MEMORIA:
                manejar_escritura_memoria(socket_fd);
                break;
            case LECTURA_PAGINA:
                manejar_lectura_pagina(socket_fd);
                break;
            case ACTUALIZACION_PAGINA:
                manejar_actualizacion_pagina(socket_fd);
                break;
            case SUSPENDER_PROCESO:
                manejar_suspension(socket_fd);
                break;
            case DESUSPENDER_PROCESO:
                manejar_desuspension(socket_fd);
                break;
            case FINALIZAR_PROCESO:
                manejar_finalizacion(socket_fd);
                break;
            case MEMORY_DUMP:
                manejar_memory_dump(socket_fd);
                break;
            default:
                log_warning(logger_memoria, "[CPU] Operación desconocida (FD: %d)", socket_fd);
                break;
        }
    }

    return NULL;
}

*/

/*
SEGUN CHAT GPT LA IMPLEM DE FUNCIONES

#include "memoria.h"
#include "memoria-log.h"
#include "memoria-admin.h"
#include "memoria-procesos.h"
#include "utils/serializacion.h"

void manejar_solicitud_instruccion(int socket_fd) {
    int pid;
    int pc;
    recv(socket_fd, &pid, sizeof(int), 0);
    recv(socket_fd, &pc, sizeof(int), 0);

    char* instruccion = obtener_instruccion(pid, pc);
    enviar_string(socket_fd, instruccion);

    log_info(logger_memoria, "## PID: %d - Obtener instruccion: %d - Instruccion: %s", pid, pc, instruccion);
    free(instruccion);
}

void manejar_lectura_memoria(int socket_fd) {
    int direccion_logica;
    int pid;
    recv(socket_fd, &pid, sizeof(int), 0);
    recv(socket_fd, &direccion_logica, sizeof(int), 0);

    char* valor = leer_memoria(pid, direccion_logica);
    enviar_string(socket_fd, valor);

    log_info(logger_memoria, "## PID: %d - Lectura - Dir. Lógica: %d - Valor: %s", pid, direccion_logica, valor);
    free(valor);
}

void manejar_escritura_memoria(int socket_fd) {
    int direccion_logica;
    int pid;
    char* valor;

    recv(socket_fd, &pid, sizeof(int), 0);
    recv(socket_fd, &direccion_logica, sizeof(int), 0);
    valor = recibir_string(socket_fd);

    escribir_memoria(pid, direccion_logica, valor);

    log_info(logger_memoria, "## PID: %d - Escritura - Dir. Lógica: %d - Valor: %s", pid, direccion_logica, valor);
    free(valor);
}

void manejar_lectura_pagina(int socket_fd) {
    int direccion_fisica;
    recv(socket_fd, &direccion_fisica, sizeof(int), 0);

    void* pagina = leer_pagina_entera(direccion_fisica);
    send(socket_fd, pagina, memoria_configs.tampagina, 0);

    log_info(logger_memoria, "## Lectura de página completa en Dir. Física: %d", direccion_fisica);
    free(pagina);
}

void manejar_actualizacion_pagina(int socket_fd) {
    int direccion_fisica;
    recv(socket_fd, &direccion_fisica, sizeof(int), 0);

    void* nueva_pagina = malloc(memoria_configs.tampagina);
    recv(socket_fd, nueva_pagina, memoria_configs.tampagina, 0);

    actualizar_pagina_entera(direccion_fisica, nueva_pagina);

    log_info(logger_memoria, "## Página actualizada en Dir. Física: %d", direccion_fisica);
    free(nueva_pagina);
}

void manejar_suspension(int socket_fd) {
    int pid;
    recv(socket_fd, &pid, sizeof(int), 0);

    suspender_proceso(pid);
    log_info(logger_memoria, "## PID: %d - Proceso suspendido", pid);
}

void manejar_desuspension(int socket_fd) {
    int pid;
    recv(socket_fd, &pid, sizeof(int), 0);

    desuspender_proceso(pid);
    log_info(logger_memoria, "## PID: %d - Proceso desuspendido", pid);
}

void manejar_finalizacion(int socket_fd) {
    int pid;
    recv(socket_fd, &pid, sizeof(int), 0);

    finalizar_proceso(pid);
    log_info(logger_memoria, "## PID: %d - Proceso finalizado", pid);
}

void manejar_memory_dump(int socket_fd) {
    int pid;
    recv(socket_fd, &pid, sizeof(int), 0);

    solicitar_memory_dump(pid);
    log_info(logger_memoria, "## PID: %d - Memory Dump solicitado", pid);
}

*/