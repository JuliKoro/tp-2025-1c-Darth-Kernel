#include "memoria.h"


// No olividar de incluir el log obligatorio: Conexión de Kernel: "## Kernel Conectado - FD del socket: <FD_DEL_SOCKET>" 

/*
Check 2:
    - Leer un archivo del directorio PATH_INSTRUCCIONES el cual va a tener las instrucciones del proceso
    - Asociarlas a su PID para que estén disponibles cunado CPU las pida
    - Tenerlas cargadas en mmemoria (puede ser una estructura tipo map<PID, lista de instrucciones>)
*/

/*
    El dia de mañana que se implemente el PCB se hace uso del Program Counter (PC) para saber que instrucción ejecutar
    y asi mismo no repetir archivos de instrucciones.
*/

t_administrador_memoria* administrador_memoria = NULL;

void inicializar_administrador_memoria() {
    if(administrador_memoria == NULL) {
        administrador_memoria = malloc(sizeof(t_administrador_memoria));
        administrador_memoria->memoria_principal = malloc(memoria_configs.tammemoria);
        administrador_memoria->marcos_libres = list_create();
        administrador_memoria->tablas_paginas = dictionary_create();
        administrador_memoria->metricas_proceso = dictionary_create();
        
        inicializar_marcos();
        inicializar_swap();
    }
}

    // Resto de inicialización...
    // 1. Reservar memoria principal según TAM_MEMORIA
    // 2. Inicializar lista de marcos libres
    // 3. Crear estructuras para tablas de páginas
    // 4. Abrir/Crear archivo swap

void inicializar_marcos() {
    if(administrador_memoria == NULL) return; // a chequear

    int total_marcos = memoria_configs.tammemoria / memoria_configs.tampagina;
    for (int i = 0; i < total_marcos; i++) {
        void* marco = administrador_memoria->memoria_principal + (i * memoria_configs.tampagina);
        list_add(administrador_memoria->marcos_libres, marco);
    }
}

void* obtener_marco_libre() {
    if (list_size(administrador_memoria->marcos_libres) > 0) {
        return list_remove(administrador_memoria->marcos_libres, 0);
    }
    // Implementar política de reemplazo (FIFO por ejemplo)
    return NULL;
}

void liberar_marco(void* marco) {
    // Liberar marco y agregarlo a lista de marcos libres
}

void actualizar_metricas(int pid, int tipo_operacion) {
    // Actualizar métricas según tipo de operación
}

void inicializar_swap() {
    if(administrador_memoria == NULL) return; //a chequear


    administrador_memoria->swap_file = fopen(memoria_configs.pathswapfile, "wb+");
    if (!administrador_memoria->swap_file) {
        log_error(logger_memoria, "No se pudo abrir el archivo swap");
        exit(EXIT_FAILURE);
    }
}

int escribir_pagina_swap(void* pagina) {
    fseek(administrador_memoria->swap_file, 0, SEEK_END);
    int pos = ftell(administrador_memoria->swap_file);
    fwrite(pagina, memoria_configs.tampagina, 1, administrador_memoria->swap_file);
    return pos;
}

void realizar_memory_dump(int pid) {
    char filename[256];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(filename, sizeof(filename), "%Y%m%d%H%M%S", t);
    
    char path_dump[512];
    snprintf(path_dump, sizeof(path_dump), "%s/%d-%s.dmp", 
             memoria_configs.dumppath, pid, filename);
    
    FILE* dump_file = fopen(path_dump, "wb");
    if (!dump_file) {
        log_error(logger_memoria, "Error al crear dump file para PID %d", pid);
        return;
    }
    
    // Implementar lógica para escribir contenido de memoria del proceso
    fclose(dump_file);
    log_info(logger_memoria, "## PID: %d - Memory Dump solicitado", pid);
}


int main(int argc, char* argv[]) {

    //const char* path = "/home/utnso/scripts/1234"; // esto despues hay que cambiarlo. 
    // Tiene que ser un pseudocódigo por cada proceso dentro de la ruta que pide el enunciado (en /untso/scripts)
    
    inicializar_configs();
    inicializar_logger_memoria();
    //inicializar_administrador_memoria(); // Nuevo



    procesos_en_memoria = dictionary_create();  // Inicializamos antes del servidor
    int socket_memoria = iniciar_servidor_memoria();
    //Hilo que va a estar recibiendo peticiones del kernel
    pthread_t hilo_peticiones_kernel;
    pthread_create(&hilo_peticiones_kernel, NULL, recibir_peticiones_kernel, (void*) &socket_memoria);
    log_info(logger_memoria, "Iniciando módulo Memoria...");
    
    pthread_join(hilo_peticiones_kernel, NULL);
    
    //cargar_procesos_en_memoria();
    dictionary_iterator(procesos_en_memoria, mostrar_proceso);
    //obtener_instruccion(1234, 1); // Estas dos lineas están simulando el pedido de una instrucción por parte de CPU
    //obtener_instruccion(1234, 4);


    //dictionary_destroy_and_destroy_elements(procesos_en_memoria, free); // Si querés liberar también instrucciones hay que hacer free de cada string
    log_destroy(logger_sockets);
    log_destroy(logger_memoria);
    config_destroy(memoria_tconfig);

    return 0;
}

