#include "instrucciones.h"
#include "memoria.h"
#include "memoria-conexiones.h"

t_dictionary* procesos_en_memoria;

int obtener_espacio_libre_mock() {
    return 1024; // Retorna 1KB como espacio libre (solo de prueba, lo está simulando)
}

t_proceso* leer_archivo_de_proceso(const char* filepath, int pid) {
    FILE* archivo = fopen(filepath, "r");
    if (!archivo) {
        log_error(logger_memoria, "No se pudo abrir el archivo del PID %d", pid);
        return NULL;
    }

    char** instrucciones = malloc(sizeof(char*) * 100); // Se puede ajustar dinámicamente luego
    int cantidad = 0;
    char linea[256];
    
    while (fgets(linea, sizeof(linea), archivo)) {
        linea[strcspn(linea, "\n")] = 0; // Remueve el salto de línea
        instrucciones[cantidad] = strdup(linea); // Copia segura
        cantidad++;
    }

    fclose(archivo);

    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->pid = pid;
    proceso->instrucciones = instrucciones;
    proceso->cantidad_instrucciones = cantidad;

    return proceso;
}

void destruir_proceso(void* proceso_void) {
    t_proceso* proceso = (t_proceso*) proceso_void;
    
    // Liberar páginas
    t_tabla_paginas* tabla = dictionary_get(administrador_memoria->tablas_paginas, string_itoa(proceso->pid));
    if (tabla) {
        // Liberar marcos y entradas swap
        dictionary_remove(administrador_memoria->tablas_paginas, string_itoa(proceso->pid));
    }
    
    // Mostrar métricas
    t_metricas_proceso* metricas = dictionary_get(administrador_memoria->metricas_proceso, string_itoa(proceso->pid));
    if (metricas) {
        log_info(logger_memoria, 
               "## PID: %d - Proceso Destruido - Métricas - Acc.T.Pag: %d; Inst.Sol.: %d; SWAP: %d; Mem.Prin.: %d; Lec.Mem.: %d; Esc.Mem.: %d",
               proceso->pid, 
               metricas->accesos_tablas_paginas,
               metricas->instrucciones_solicitadas,
               metricas->bajadas_swap,
               metricas->subidas_memoria,
               metricas->lecturas_memoria,
               metricas->escrituras_memoria);
    }
    
    free(proceso->instrucciones);
    free(proceso);
}

int cargar_proceso(int pid, const char* nombre_archivo) {
    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", memoria_configs.pathinstrucciones, nombre_archivo);

    t_proceso* proceso = leer_archivo_de_proceso(fullpath, pid);
    if (proceso != NULL) {
        char* pid_key = malloc(10);
        sprintf(pid_key, "%d", pid);
        dictionary_put(procesos_en_memoria, pid_key, proceso);
        log_info(logger_memoria, "## PID %d - Proceso Creado - Tamaño: <TAMAÑO>", pid);
        return 0;
    }
    return -1;
}

void cargar_procesos_en_memoria() {
    if (procesos_en_memoria == NULL) {
        procesos_en_memoria = dictionary_create();
    }


    DIR* dir = opendir(memoria_configs.pathinstrucciones);
    if (!dir) {
        log_error(logger_memoria, "No se pudo abrir el directorio de instrucciones");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Solo archivos regulares
            log_info(logger_memoria, "Leyendo archivo: %s", entry->d_name);
            int pid = atoi(entry->d_name); // Convierte nombre de archivo a PID
            if (pid == 0 && strcmp(entry->d_name, "0") != 0)
                continue; // No es un número válido

            cargar_proceso(pid, entry->d_name);
 
        }
    }

    closedir(dir);
}

void mostrar_proceso(char* key, void* value) {
    t_proceso* proceso = (t_proceso*)value;
    printf("PID: %d\n", proceso->pid);
    for (int i = 0; i < proceso->cantidad_instrucciones; i++) {
        printf("  - %s\n", proceso->instrucciones[i]);
    }
}

const char* obtener_instruccion(int pid, int pc) {
    char pid_key[10];
    sprintf(pid_key, "%d", pid);

    t_proceso* proceso = dictionary_get(procesos_en_memoria, pid_key);
    if (proceso != NULL && pc <= proceso->cantidad_instrucciones) {
        const char* instruccion = proceso->instrucciones[pc-1];
        log_info(logger_memoria, "## PID %d - Obtener instrucción: %d - Instrucción: %s", pid, pc, instruccion);
        return instruccion;
    }

    log_info(logger_memoria, "No se encontró el proceso con PID %d o PC fuera de rango", pid);

    return NULL;
}

void atender_peticion_kernel(void* socket_cliente){
    int socket_fd = *(int*) socket_cliente;
    free(socket_cliente);

    id_modulo_t modulo_recibido;

    if(recibir_handshake(socket_fd, &modulo_recibido) == -1){
        log_error(logger_sockets, "[HANDSHAKE] Error en recepcion de handshake. Cierro conexion.");
        close(socket_fd);
        return;
    }

    log_info(logger_memoria, "[HANDSHAKE] Handshake recibido correctamente. Conexion establecida con kernel");

    
        //Recibir pid, tamanio, pc, archivo pseudocodigo
        t_paquete* paquete = recibir_paquete(socket_fd);
        t_pcb* pcb = deserializar_pcb(paquete->buffer);
        liberar_paquete(paquete);

        //Ver si puedo cargar instrucciones en memoria
        if(cargar_proceso(pcb->pid, pcb->archivo_pseudocodigo) == -1){
            log_error(logger_memoria, "[CARGAR PROCESO] Error al cargar proceso. PID: %d", pcb->pid);
            enviar_bool(socket_fd, false);
        }

    //Enviar respuesta al kernel
    enviar_bool(socket_fd, true);      
    
    close(socket_fd);
}

void* recibir_peticiones_kernel(void* socket_memoria){
    int socket_fd = *(int*) socket_memoria;
    //free(socket_memoria);
    while(true){
        int cliente_fd = esperar_cliente(socket_fd);

        pthread_t hilo_cliente;
        int* socket_cliente = malloc(sizeof(int));
        *socket_cliente = cliente_fd;

        pthread_create(&hilo_cliente, NULL, (void*)atender_peticion_kernel, (void*) socket_cliente);
        pthread_detach(hilo_cliente);
    }
}