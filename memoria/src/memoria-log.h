/**
 * @brief Cabecera para la gestión de logs del módulo de Memoria.
 *
 * Declara la variable global `logger_memoria` que será utilizada para
 * registrar eventos y mensajes en todo el módulo. También define los
 * prototipos de las funciones para inicializar y destruir este logger.
 */

 #ifndef MEMORIA_LOG_H_
 #define MEMORIA_LOG_H_
 
 #include <commons/log.h>
 #include "memoria-configs.h" // Necesario para acceder a memoria_configs.loglevel
 #include <utils/sockets.h>
 #include <utils/serializacion.h>
 
 // Logger global que sera usado en todo el modulo
 extern t_log* logger_memoria; // Logger principal del módulo de memoria
 
 /**
  * @brief Inicializa el logger del módulo de memoria.
  *
  * Crea un logger con el nombre de archivo "memoria.log", el nombre de programa "MEMORIA",
  * y el nivel de log definido en las configuraciones (`memoria_configs.loglevel`).
  */
 void inicializar_logger_memoria();
 
 /**
  * @brief Destruye el logger del módulo de memoria.
  *
  * Libera los recursos asociados al logger si este ha sido inicializado.
  */
 void destruir_logger_memoria();
 
 #endif /* MEMORIA_LOG_H_ */
 
 