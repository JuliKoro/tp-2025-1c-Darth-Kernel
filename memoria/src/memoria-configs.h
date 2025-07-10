/**
 * @brief Cabecera para la gestión de configuraciones del módulo de Memoria.
 *
 * Define la estructura `memoriaconfigs` para almacenar los parámetros de configuración
 * leídos del archivo `memoria.config`. Declara la variable global `memoria_configs`
 * y el prototipo de la función `inicializar_configs` para cargar estos valores.
 */

 #ifndef MEMORIA_CONFIGS_H
 #define MEMORIA_CONFIGS_H
 
 #include <utils/configs.h> // Incluye la librería de utilidades para manejo de configs
 
 // Variable global para el objeto de configuración de la librería commons
 extern t_config* memoria_tconfig;
 
 // Estructura para almacenar los parámetros de configuración de la memoria
 typedef struct memoriaconfigs {
     int puertoescucha;
     int tammemoria;
     int tampagina;
     int entradasportabla;
     int cantidadniveles;
     int retardomemoria;
     char* pathswapfile;
     int retardoswap;
     char* loglevel;
     char* dumppath;
     char* pathinstrucciones;
 } memoriaconfigs;
 
 // Variable global que contendrá todas las configuraciones cargadas
 extern memoriaconfigs memoria_configs;
 
 /**
  * @brief Inicializa las configuraciones del módulo de memoria.
  *
  * Lee el archivo de configuración "memoria.config" y carga los valores
  * en la estructura global `memoria_configs`.
  */
 void inicializar_configs();
 
 #endif /* MEMORIA_CONFIGS_H */
 
 