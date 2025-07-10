/**
 * @brief Implementación de la carga de configuraciones del módulo de Memoria.
 *
 * Contiene la definición de las variables globales `memoria_tconfig` y `memoria_configs`,
 * y la implementación de la función `inicializar_configs` que se encarga de leer
 * el archivo de configuración y poblar la estructura `memoriaconfigs`.
 */

 #include "memoria-configs.h"
 #include "memoria-log.h" // Para usar el logger en caso de errores
 
 // Inicializo los config y el struct global
 t_config* memoria_tconfig;
 memoriaconfigs memoria_configs;
 
 /**
  * @brief Inicializa las configuraciones del módulo de memoria.
  *
  * Lee el archivo de configuración "memoria.config" y carga los valores
  * en la estructura global `memoria_configs`. En caso de error al abrir
  * el archivo o al leer alguna clave, registra un error y termina la ejecución.
  */
 void inicializar_configs() {
     // Creo un config para memoria
     memoria_tconfig = iniciar_config("memoria.config");
     if (memoria_tconfig == NULL) {
         // Si el logger aún no está inicializado, se imprime directamente a stderr
         fprintf(stderr, "Error: No se pudo abrir el archivo de configuración 'memoria.config'.\n");
         exit(EXIT_FAILURE);
     }
 
     // Creo el struct donde voy a cargar los datos
     memoriaconfigs configcargado;
 
     // Cargo los datos en el, usando las funciones get
     // Se utilizan funciones auxiliares para cargar variables y manejar errores
     configcargado.puertoescucha = cargar_variable_int(memoria_tconfig, "PUERTO_ESCUCHA");
     configcargado.tammemoria = cargar_variable_int(memoria_tconfig, "TAM_MEMORIA");
     configcargado.tampagina = cargar_variable_int(memoria_tconfig, "TAM_PAGINA");
     configcargado.entradasportabla = cargar_variable_int(memoria_tconfig, "ENTRADAS_POR_TABLA");
     configcargado.cantidadniveles = cargar_variable_int(memoria_tconfig, "CANTIDAD_NIVELES");
     configcargado.retardomemoria = cargar_variable_int(memoria_tconfig, "RETARDO_MEMORIA");
     configcargado.pathswapfile = cargar_variable_string(memoria_tconfig, "PATH_SWAPFILE");
     configcargado.retardoswap = cargar_variable_int(memoria_tconfig, "RETARDO_SWAP");
     configcargado.loglevel = cargar_variable_string(memoria_tconfig, "LOG_LEVEL");
     configcargado.dumppath = cargar_variable_string(memoria_tconfig, "DUMP_PATH");
     configcargado.pathinstrucciones = cargar_variable_string(memoria_tconfig, "PATH_INSTRUCCIONES");
 
     // Igualo el struct global a este, de esta forma puedo usar los datos en cualquier archivo del modulo
     memoria_configs = configcargado;
 
     // Opcional: Loguear las configuraciones cargadas para verificación
     // Esto se puede hacer una vez que el logger_memoria esté inicializado
     // log_info(logger_memoria, "Configuraciones cargadas:");
     // log_info(logger_memoria, "  PUERTO_ESCUCHA: %d", memoria_configs.puertoescucha);
     // ...
 }
 
 
