#ifndef SEMAFOROS_H
#define SEMAFOROS_H

#include <semaphore.h>
#include <pthread.h>

extern sem_t sem_memoria_disponible;


/**
 * @brief Inicializa los semaforos globales
 * 
 * @return void
 */
void inicializar_semaforos_globales();

#endif
