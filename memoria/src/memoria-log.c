/**
 * @brief Implementación de la gestión de logs del módulo de Memoria.
 *
 * Contiene la definición de la variable global `logger_memoria` y la
 * implementación de las funciones para inicializar y destruir el logger,
 * permitiendo un registro centralizado de eventos y errores en el módulo.
 */

 #include "memoria-log.h"

 // Definición de la variable global logger_memoria
 t_log* logger_memoria;
 
 /**
  * @brief Inicializa el logger del módulo de memoria.
  *
  * Crea un logger con el nombre de archivo "memoria.log", el nombre de programa "MEMORIA",
  * y el nivel de log definido en las configuraciones (`memoria_configs.loglevel`).
  */
 void inicializar_logger_memoria() {
     // log_create(file, program_name, is_active_console, level)
     logger_memoria = log_create("memoria.log", "MEMORIA", true, log_level_from_string(memoria_configs.loglevel));
     if (logger_memoria == NULL) {
         fprintf(stderr, "Error: No se pudo inicializar el logger de memoria.\n");
         exit(EXIT_FAILURE);
     }
     log_info(logger_memoria, "Logger de memoria inicializado con nivel: %s", memoria_configs.loglevel);
 }
 
 /**
  * @brief Destruye el logger del módulo de memoria.
  *
  * Libera los recursos asociados al logger si este ha sido inicializado.
  */
 void destruir_logger_memoria() {
     // Chequeo que el logger exista y lo destruyo
     if (logger_memoria != NULL) {
         log_destroy(logger_memoria);
         logger_memoria = NULL; // Para evitar doble liberación
     }
 }
 
 
