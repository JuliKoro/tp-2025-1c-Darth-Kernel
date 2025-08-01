#include "memoria.h"
#include "memoria-swap.h"

 /**
  * @brief Inicializa el archivo de SWAP y su bitmap.
  *
  * Abre o crea el archivo especificado en `PATH_SWAPFILE`. Si es nuevo, lo llena
  * con ceros. Calcula el número total de páginas en SWAP y crea un `t_bitarray`
  * para gestionar las posiciones libres/ocupadas.
  */
void inicializar_swap() {
    if (administrador_memoria == NULL) {
        log_error(logger_memoria, "Administrador de memoria no inicializado antes de inicializar SWAP.");
        return;
    }

    // Eliminar archivo SWAP anterior si existe
    if (access(memoria_configs.pathswapfile, F_OK) == 0) {
        if (remove(memoria_configs.pathswapfile) == 0) {
            log_info(logger_memoria, "Archivo SWAP anterior eliminado: %s", memoria_configs.pathswapfile);
        } else {
            log_warning(logger_memoria, "No se pudo eliminar el archivo SWAP existente: %s", memoria_configs.pathswapfile);
        }
    }

    // Crear nuevo archivo de swap
    administrador_memoria->swap_file = fopen(memoria_configs.pathswapfile, "wb+");
    if (!administrador_memoria->swap_file) {
        log_error(logger_memoria, "No se pudo crear el archivo SWAP en %s. Verifique permisos o ruta.", memoria_configs.pathswapfile);
        exit(EXIT_FAILURE);
    }

    // Tamaño dinámico = 2x memoria principal
    long initial_swap_size = memoria_configs.tammemoria * 2;

    // Alineación con tamaño de página
    if (initial_swap_size % memoria_configs.tampagina != 0) {
        initial_swap_size = ((initial_swap_size / memoria_configs.tampagina) + 1) * memoria_configs.tampagina;
    }

    // Inicializar archivo con ceros
    char* zero_buffer = calloc(1, initial_swap_size);
    if (zero_buffer == NULL) {
        log_error(logger_memoria, "Error al asignar buffer para inicializar SWAP.");
        fclose(administrador_memoria->swap_file);
        exit(EXIT_FAILURE);
    }

    fwrite(zero_buffer, 1, initial_swap_size, administrador_memoria->swap_file);
    free(zero_buffer);
    fflush(administrador_memoria->swap_file);
    rewind(administrador_memoria->swap_file);

    log_info(logger_memoria, "Archivo SWAP creado limpio. Tamaño: %ld bytes (2x memoria principal).", initial_swap_size);

    // 📐 Calcular número total de páginas en SWAP
    long file_size = initial_swap_size;
    int total_paginas_swap = file_size / memoria_configs.tampagina;

    if (total_paginas_swap == 0) {
        log_warning(logger_memoria, "El archivo de SWAP está vacío o es demasiado pequeño. Se ajusta tamaño mínimo.");
        total_paginas_swap = 1;
    }

    // 🧠 Inicializar bitmap
    int bitmap_size_bytes = (total_paginas_swap + 7) / 8;
    void* bitmap_buffer = calloc(1, bitmap_size_bytes);
    if (bitmap_buffer == NULL) {
        log_error(logger_memoria, "Error al asignar memoria para el bitmap de SWAP.");
        fclose(administrador_memoria->swap_file);
        exit(EXIT_FAILURE);
    }

    if (bitmap_swap) {
        bitarray_destroy(bitmap_swap); // destruir anterior si existía
    }

    bitmap_swap = bitarray_create_with_mode(bitmap_buffer, bitmap_size_bytes, LSB_FIRST);
    log_info(logger_memoria, "Bitmap de SWAP inicializado en limpio. Total de páginas: %d.", total_paginas_swap);
}


  /**
  * @brief Aplica un retardo basado en `RETARDO_SWAP`.
  *
  * Utiliza `usleep` para pausar la ejecución por el tiempo configurado
  * en milisegundos.
  */
 void aplicar_retardo_swap() {
    if (memoria_configs.retardoswap > 0) {
        usleep(memoria_configs.retardoswap * 1000); // Convertir ms a microsegundos
    }
}