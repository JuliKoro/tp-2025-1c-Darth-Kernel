#include "acceso_memoria.h"

int escribir_en_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio, void* dato, int socket_memoria) {
    t_escritura_memoria* pagina_escrita = malloc(sizeof(t_escritura_memoria));
    pagina_escrita->pid = pid;
    pagina_escrita->direccion_fisica = direccion_fisica;
    pagina_escrita->tamanio = tamanio;
    pagina_escrita->dato = dato;

    t_buffer* buffer = serializar_escritura_memoria(pagina_escrita);
    t_paquete* paquete = empaquetar_buffer(PAQUETE_WRITE, buffer);

    free(pagina_escrita);

    if (enviar_paquete(socket_memoria, paquete) == -1) {
        log_error(logger_cpu, "Error al enviar escritura de datos a Memoria.");
        return -1; // Manejo de error
    }

    char* respuesta_memoria = recibir_mensaje(socket_memoria);
    if (strcmp(respuesta_memoria, "OK") != 0) { // REVISAR SI ESTA BIEN
        log_error(logger_cpu, "Error en la escritura de datos en Memoria.");
        return -1; // Manejo de error
    }

    log_debug(logger_cpu, "Escritura en Memoria exitosa!");

    log_info(logger_cpu, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %s",pid, direccion_fisica, (char*)dato);

    return 0;
}

void* leer_de_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tamanio, int socket_memoria) {
    // Creo y serializo la solicitud para leer un dato de Memoria
    t_lectura_memoria* solicitud = malloc(sizeof(t_lectura_memoria));
    solicitud->pid = pid;
    solicitud->direccion_fisica = direccion_fisica;
    solicitud->tamanio = tamanio; // para solicitar una pagina para la Cache se usa el tamanio de la pagina completa
    

    t_buffer* buffer_solicitud = serializar_lectura_memoria(solicitud);
    t_paquete* paquete_solicitud = empaquetar_buffer(PAQUETE_READ, buffer_solicitud);

    free(solicitud);

    if (enviar_paquete(socket_memoria, paquete_solicitud) == -1) {
        log_error(logger_cpu, "Error al enviar solicitud de lectura de datos de Memoria.");
        return NULL; // Manejo de error
    }

    // Recibo la respuesta de memoria
    void* datos = recibir_mensaje(socket_memoria);

    if (strcmp(datos, "") == 0) {
        log_error(logger_cpu, "Error al recibir la respuesta de Memoria para la lectura.");
        return NULL; // Manejo de error
    }

    log_info(logger_cpu, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %s", pid, direccion_fisica, (char*)datos);

    return datos;
}