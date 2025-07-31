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

    // Abrir o crear el archivo de swap
    administrador_memoria->swap_file = fopen(memoria_configs.pathswapfile, "rb+");
    if (!administrador_memoria->swap_file) {
        log_debug(logger_memoria, "Archivo SWAP no encontrado en %s. Creando nuevo archivo.", memoria_configs.pathswapfile);
        administrador_memoria->swap_file = fopen(memoria_configs.pathswapfile, "wb+");
        if (!administrador_memoria->swap_file) {
            log_error(logger_memoria, "No se pudo crear/abrir el archivo SWAP en %s. Verifique permisos o ruta.", memoria_configs.pathswapfile);
            exit(EXIT_FAILURE);
        }
        
        // CORRECCIÓN: Tamaño dinámico = 2x memoria principal
        long initial_swap_size = memoria_configs.tammemoria * 2;
        
        // Asegurar alineación con tamaño de página
        if (initial_swap_size % memoria_configs.tampagina != 0) {
            initial_swap_size = ((initial_swap_size / memoria_configs.tampagina) + 1) * memoria_configs.tampagina;
        }
        
        char* zero_buffer = calloc(1, initial_swap_size);
        if (zero_buffer == NULL) {
            log_error(logger_memoria, "Error al asignar buffer para inicializar SWAP.");
            fclose(administrador_memoria->swap_file);
            exit(EXIT_FAILURE);
        }
        
        fwrite(zero_buffer, 1, initial_swap_size, administrador_memoria->swap_file);
        free(zero_buffer);
        fflush(administrador_memoria->swap_file);
        log_debug(logger_memoria, "Archivo SWAP creado con tamaño dinámico de %ld bytes (2x memoria principal).", initial_swap_size);
    } else {
        log_debug(logger_memoria, "Archivo SWAP existente abierto en %s.", memoria_configs.pathswapfile);
    }

    // Obtener el tamaño actual del archivo para calcular el número de páginas
    fseek(administrador_memoria->swap_file, 0, SEEK_END);
    long file_size = ftell(administrador_memoria->swap_file);
    rewind(administrador_memoria->swap_file); // Volver al inicio del archivo

    int total_paginas_swap = 0;
    if (memoria_configs.tampagina > 0) {
        total_paginas_swap = file_size / memoria_configs.tampagina;
    } else {
        log_error(logger_memoria, "Tamaño de página (TAM_PAGINA) es cero o inválido. No se puede inicializar SWAP.");
        fclose(administrador_memoria->swap_file);
        exit(EXIT_FAILURE);
    }

    if (total_paginas_swap == 0) {
        log_warning(logger_memoria, "El archivo de SWAP está vacío o es demasiado pequeño. Asegúrese de que tenga un tamaño inicial adecuado.");
        // Si el archivo está vacío, se puede decidir un tamaño mínimo para el bitmap
        total_paginas_swap = 1; // Mínimo para evitar errores de bitarray si el archivo es 0 bytes
    }

    // Inicializar bitmap para administración de swap
    // El tamaño del bitarray en bytes debe ser suficiente para todos los bits
    int bitmap_size_bytes = (total_paginas_swap + 7) / 8; // Redondeo hacia arriba
    void* bitmap_buffer = calloc(1, bitmap_size_bytes); // Inicializa con ceros
    if (bitmap_buffer == NULL) {
        log_error(logger_memoria, "Error al asignar memoria para el bitmap de SWAP.");
        fclose(administrador_memoria->swap_file);
        exit(EXIT_FAILURE);
    }
    bitmap_swap = bitarray_create_with_mode(
        bitmap_buffer,
        bitmap_size_bytes,
        LSB_FIRST // O MSB_FIRST, según la convención deseada
    );
    // Todos los bits ya están en 0 (libres) debido a calloc
    log_debug(logger_memoria, "Bitmap de SWAP inicializado. Total de páginas SWAP: %d.", total_paginas_swap);
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