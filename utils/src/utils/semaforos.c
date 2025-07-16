#include "semaforos.h"
sem_t sem_memoria_disponible;
void inicializar_semaforos_globales() {
    sem_init(&sem_memoria_disponible, 0, 1);
}