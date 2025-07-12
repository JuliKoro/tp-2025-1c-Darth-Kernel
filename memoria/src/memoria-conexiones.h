/**
 * @brief Cabecera para la gestión de conexiones del módulo de Memoria.
 *
 * Define los prototipos de las funciones relacionadas con la inicialización
 * del servidor de memoria y la atención de peticiones de otros módulos
 * (Kernel y CPU).
 */

 #ifndef MEMORIA_CONEXIONES_H_
 #define MEMORIA_CONEXIONES_H_
 
 #include <utils/sockets.h> // Incluye la librería de utilidades para sockets
 #include "memoria-configs.h" // Necesario para acceder a memoria_configs.puertoescucha
 #include "memoria-log.h"     // Necesario para usar el logger
 
 /**
  * @brief Inicia el servidor de memoria.
  *
  * Configura y levanta un servidor TCP en el puerto especificado en las configuraciones.
  * Inicializa un logger específico para las conexiones.
  *
  * @return El file descriptor del socket del servidor, o -1 en caso de error.
  */
 int iniciar_servidor_memoria();

 // Prototipos para manejo de conexiones con Kernel. Están en instrucciones.h, ahí está la explicacion tambien de porqué
 void* recibir_peticiones_kernel(void* socket_memoria);
 void atender_peticion_kernel(void* socket_cliente);
 
 #endif /* MEMORIA_CONEXIONES_H_ */

 /*
 void* recibir_peticiones_cpu(void* socket_memoria);
void* atender_peticion_cpu(void* socket_cliente);
 */
 
 