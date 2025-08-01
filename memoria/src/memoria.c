/**
 * @brief Archivo principal del módulo de Memoria.
 *
 * Este archivo contiene la función `main` que inicializa el módulo de Memoria,
 * carga las configuraciones, inicializa los loggers, el administrador de memoria,
 * y levanta los servidores para atender peticiones del Kernel y la CPU.
 * También se encarga de la liberación de recursos al finalizar.
 */

 #include "memoria.h"
 #include "memoria-procesos.h" // <--- ¡Añadir esta línea! Necesaria para t_proceso y procesos_en_memoria
 #include "memoria-conexiones.h"
 #include "memoria-admin.h"
 #include <pthread.h>

 
 // Variables globales definidas en memoria.h
 //t_administrador_memoria* administrador_memoria = NULL;
 //t_bitarray* bitmap_swap = NULL;
 

//--------------------------------------------------------------------------------------

 /**
 * @brief Archivo principal del módulo de Memoria.
 *
 * Esta función contiene la inicialización del módulo de Memoria,
 * carga las configuraciones, inicializa los loggers, el administrador de memoria,
 * y levanta los servidores para atender peticiones del Kernel y la CPU.
 * También se encarga de la liberación de recursos al finalizar.
 */

 int main(int argc, char* argv[]) {
    // Inicializa las configuraciones del módulo de memoria.
    inicializar_configs();
    // Inicializa el logger específico del módulo de memoria.
    inicializar_logger_memoria();

    // Verifica que las configuraciones esenciales de memoria sean válidas.
    if (memoria_configs.tammemoria <= 0 || memoria_configs.tampagina <= 0) {
        log_error(logger_memoria, "Configuración de memoria inválida: TAM_MEMORIA o TAM_PAGINA son cero o negativos.");
        exit(EXIT_FAILURE);
    }

    // Inicializa el administrador de memoria, incluyendo la memoria principal, marcos, tablas de páginas y SWAP.
    inicializar_administrador_memoria();

    // Inicia el servidor de memoria para escuchar conexiones entrantes.
    int socket_servidor = iniciar_servidor_memoria();
    if (socket_servidor == -1) {
        log_error(logger_memoria, "No se pudo iniciar el servidor de memoria. Abortando.");
        exit(EXIT_FAILURE);
    }
    // Log obligatorio: Conexión de Kernel.
    //log_info(logger_memoria, "## Kernel Conectado - FD del socket: %d", socket_servidor);

    // Crea un hilo para atender las peticiones provenientes del módulo Kernel.
    pthread_t hilo_conexiones;
    // Se pasa una copia del socket_servidor para evitar problemas de concurrencia si se modifica.
    pthread_create(&hilo_conexiones, NULL, escuchar_peticiones, (void*)(intptr_t)socket_servidor);

    // Crea un hilo para atender las peticiones provenientes de las múltiples CPU.
    //pthread_t hilo_cpus;
    // Se pasa una copia del socket_servidor para evitar problemas de concurrencia si se modifica.
    //pthread_create(&hilo_cpus, NULL, recibir_peticiones_cpu, (void*)&socket_servidor);

    // Espera a que los hilos de Kernel y CPU finalicen (en un sistema real, estos hilos suelen ser infinitos).
    pthread_detach(hilo_conexiones);

    while(true){
    }
    //pthread_join(hilo_cpus, NULL);
    // Libera todos los recursos utilizados por el módulo de memoria.
    // Destruye el diccionario de procesos en memoria y sus elementos asociados.
            //dictionary_destroy_and_destroy_elements(procesos_en_memoria, destruir_proceso);
    if (procesos_en_memoria != NULL) {
        dictionary_destroy_and_destroy_elements(procesos_en_memoria, destruir_proceso);
        log_debug(logger_memoria, "Diccionario de procesos en memoria destruido.");
    } else {
        log_warning(logger_memoria, "El diccionario de procesos en memoria ya estaba NULL.");
        }

    // Destruye el administrador de memoria y libera la memoria principal, marcos, tablas de páginas y SWAP.
    destruir_administrador_memoria();
    // Destruye el logger del módulo de memoria.
    destruir_logger_memoria();
    // Destruye la configuración cargada.
    config_destroy(memoria_tconfig);


    return 0;
}
 
 