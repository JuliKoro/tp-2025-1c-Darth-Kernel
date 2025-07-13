#ifndef MANEJO_CPU_H
#define MANEJO_CPU_H

#include "planificacion.h"
#include "kernel-conexiones.h"
#include <kernel-log.h>
#include <utils/listas.h>
#include <utils/serializacion.h>
#include <stdint.h>


/*


*/
/**
 * @brief Inicia ambos receptores, de dispatch y de interrupt.
 * 
 * @return void* 
 */
void* iniciar_receptores_cpu();


/**
 * @brief Receptor de mensajes de dispatch.
 * 
 * @param socket_cpu_dispatch Socket de la CPU.
 * @return void* 
 */
void* receptor_dispatch(void* socket_cpu_dispatch);

/**
 * @brief Receptor de mensajes de interrupt.
 * 
 * @param socket_cpu_interrupt Socket de la CPU.
 * @return void* 
 */
void* receptor_interrupt(void* socket_cpu_interrupt);


/**
 * @brief Si la CPU esta en la lista, actualiza el socket de dispatch. Si no esta, la crea con el socket de dispatch y la agerga a la lista
 * 
 * @param socket_cpu_dispatch Socket de la CPU.
 * @param id_cpu ID de la CPU.
 * @return void* 
 */
void* guardar_cpu_dispatch(void* socket_cpu_dispatch, int id_cpu);

/**
 * @brief Si la CPU esta en la lista, actualiza el socket de interrupt. Si no esta, la crea con el socket de interrupt y la agerga a la lista
 * 
 * @param socket_cpu_interrupt Socket de la CPU.
 * @param id_cpu ID de la CPU.
 * @return void* 
 */
void* guardar_cpu_interrupt(void* socket_cpu_interrupt, int id_cpu);


/**
 * @brief Maneja el socket de dispatch. Recibe mensajes de la CPU y los maneja.
 * 
 * @param socket_cpu_dispatch Socket de la CPU.
 * @return void* 
 */
void* manejo_dispatch(void* socket_cpu_dispatch);


/**
 * @brief Maneja el socket de interrupt. Recibe mensajes de la CPU y los maneja.
 * 
 * @param socket_cpu_interrupt Socket de la CPU.
 * @return void* 
 */
void* manejo_interrupt(void* socket_cpu_interrupt);


/**
 * @brief Elimina la CPU de la lista de CPUs.
 * 
 * @param socket Socket de la CPU, puede ser de dispatch o de interrupt.
 * @return void* 
 */
void* eliminar_cpu_por_socket(int socket);


#endif
