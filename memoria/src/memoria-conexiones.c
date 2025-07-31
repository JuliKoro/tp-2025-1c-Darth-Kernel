#include "memoria-conexiones.h"
#include "memoria-procesos.h"
#include "memoria-operaciones.h"


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
                //enviar_bool(socket_fd, false);
                enviar_mensaje('\0', socket_fd); // CORROBORAR SI SIRVE EL \0
            } else {
                //enviar_bool(socket_fd, true);
                enviar_mensaje(memoria_leida, socket_fd); 
            }
        }
//int pid, int direccion_fisica, int tam, void* valor
        if(paquete->codigo_operacion == PAQUETE_WRITE){
            t_escritura_memoria* datos_escritura_memoria = deserializar_escritura_memoria(paquete->buffer);
            if(escribir_memoria(datos_escritura_memoria->pid, datos_escritura_memoria->direccion_fisica, 
                datos_escritura_memoria->tamanio, datos_escritura_memoria->dato) == false){
                log_error(logger_memoria, "[ESCRITURA MEMORIA] No se pudo escribir la memoria del proceso %d y direccion fisica %d",
                    datos_escritura_memoria->pid, datos_escritura_memoria->direccion_fisica);
                //enviar_bool(socket_fd, false);
                enviar_mensaje("ERROR", socket_fd);
            } else {
                //enviar_bool(socket_fd, true);
                enviar_mensaje("OK", socket_fd);
            }
        }
        //Enviar respuesta al kernel     
        liberar_paquete(paquete);
        close(socket_fd);
        return NULL;
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
            log_info(logger_memoria, "## PID: %d - Memory Dump solicitado", pcb->pid);
            //ver si puedo suspender proceso de memoria
            if(realizar_memory_dump(pcb->pid) == -1){
                log_error(logger_memoria, "[MEMORIA DUMP PROCESO] Error al dumpear proceso. PID: %d", pcb->pid);
            enviar_bool(socket_fd, false);
            } else {
                enviar_bool(socket_fd, true);
            }
        }
        return NULL;
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
            log_info(logger_memoria, "## Kernel Conectado - FD del socket: %d", cliente_fd);
            pthread_t hilo_cliente;
            pthread_create(&hilo_cliente, NULL, manejar_conexion_kernel, (void*)(intptr_t) cliente_fd);
            pthread_detach(hilo_cliente);
          }

        
    }
}
