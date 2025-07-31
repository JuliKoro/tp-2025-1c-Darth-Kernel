#include "memoria.h"
#include "memoria-marcos.h"
#include "memoria-swap.h"
#include "memoria-procesos.h"

 /**
  * @brief Inicializa la lista de marcos libres.
  *
  * Divide la memoria principal en marcos de tamaño `tampagina` y los añade
  * a la lista de `marcos_libres`.
  */
 void inicializar_marcos() {
    if (administrador_memoria == NULL) {
        log_error(logger_memoria, "Administrador de memoria no inicializado antes de inicializar marcos.");
        return;
    }

    int total_marcos = memoria_configs.tammemoria / memoria_configs.tampagina;
    for (int i = 0; i < total_marcos; i++) {
        void* marco = (char*)administrador_memoria->memoria_principal + (i * memoria_configs.tampagina);
        list_add(administrador_memoria->marcos_libres, marco);
    }
    log_debug(logger_memoria, "Marcos de memoria inicializados. Total: %d, Tamaño de página: %d bytes.", total_marcos, memoria_configs.tampagina);
}

 /**
  * @brief Obtiene un marco de memoria libre.
  *
  * Primero intenta obtener un marco de la lista de `marcos_libres`.
  * Si no hay marcos libres, aplica el algoritmo FIFO para seleccionar un marco
  * ocupado, lo swapea si es necesario y lo devuelve.
  *
  * @return Puntero al marco de memoria libre u ocupado (si se desalojó), o NULL si no hay marcos disponibles.
  */
 void* obtener_marco_libre() {
    if (list_size(administrador_memoria->marcos_libres) > 0) {
        void* marco = list_remove(administrador_memoria->marcos_libres, 0);
        list_add(administrador_memoria->marcos_ocupados, marco);
        log_debug(logger_memoria, "Marco libre obtenido de la lista de libres. Dirección: %p", marco);
        return marco;
    }

    // Algoritmo de reemplazo FIFO: seleccionar el primer marco ocupado
    if (list_size(administrador_memoria->marcos_ocupados) > 0) {
        void* marco_victima = list_remove(administrador_memoria->marcos_ocupados, 0);
        t_marco_info info = buscar_marco_en_tablas(marco_victima);

        if (info.pid != -1) {
            log_debug(logger_memoria, "Aplicando FIFO: Marco víctima para PID %d en dirección %p.", info.pid, marco_victima);
            // Escribir en swap si está modificada
            if (info.entrada->modificado) {
                if (info.entrada->posicion_swap == -1) {
                    info.entrada->posicion_swap = obtener_posicion_libre_swap();
                    if (info.entrada->posicion_swap == -1) {
                        log_error(logger_memoria, "No hay espacio libre en SWAP para desalojar marco de PID %d. Fallo crítico.", info.pid);
                        // Devolver el marco a la lista de ocupados si no se puede swapear
                        list_add_in_index(administrador_memoria->marcos_ocupados, 0, marco_victima);
                        return NULL; // No se pudo obtener un marco
                    }

                    actualizar_metricas(info.pid, BAJADA_SWAP); // Métrica de bajada a SWAP
                    log_debug(logger_memoria, "Página de PID %d swapeada a posición %d.", info.pid, info.entrada->posicion_swap);
                }
                escribir_pagina_swap(info.entrada->posicion_swap, marco_victima);
                info.entrada->modificado = false;
                aplicar_retardo_swap();
            }

            //Liberar marco
            liberar_marco(marco_victima);
            log_debug(logger_memoria, "Marco %p liberado correctamente", marco_victima);

            // Obtener nuevo marco libre
            if (list_size(administrador_memoria->marcos_libres) > 0) {
                void* nuevo_marco = list_remove(administrador_memoria->marcos_libres, 0);
                list_add(administrador_memoria->marcos_ocupados, nuevo_marco);
                return nuevo_marco;
            }

        } else {
            log_warning(logger_memoria, "Marco víctima %p no asociado a ninguna entrada de página. Posible error lógico.", marco_victima);
        }
    }

    log_error(logger_memoria, "No hay marcos disponibles en memoria principal.");
    return NULL; // No hay marcos disponibles
}
