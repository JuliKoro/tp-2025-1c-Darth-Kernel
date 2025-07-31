#ifndef MEMORIA_TABLAS_H
#define MEMORIA_TABLAS_H

#include "memoria.h"

/**
 * @brief Cabecera para la gestión de tablas de páginas.
 *
 * Declara las funciones relacionadas con la creación y destrucción de tablas de páginas,
 * traducción de direcciones lógicas a físicas, y manejo de fallos de página.
 */

/**
 * @brief Crea una nueva tabla de páginas para un nivel específico.
 *
 * Asigna memoria para la estructura `t_tabla_nivel` y para sus entradas.
 * Inicializa cada entrada como no presente, no modificada, sin marco asignado
 * y sin posición en SWAP. Si no es el último nivel, crea recursivamente
 * la tabla de páginas del siguiente nivel y almacena su dirección en el campo `marco`.
 *
 * @param nivel Nivel de la tabla de páginas a crear (0 para el primer nivel).
 * @return Puntero a la tabla de páginas creada.
 */
t_tabla_nivel* crear_tabla_nivel(int nivel);

/**
 * @brief Destruye una tabla de páginas y sus sub-tablas recursivamente.
 *
 * Libera la memoria de todas las entradas de la tabla y de la propia tabla.
 * Si una entrada apunta a una página de datos presente, libera el marco asociado.
 * Si una entrada tiene una posición en SWAP, la libera.
 * Si es una tabla de nivel intermedio, llama recursivamente a `destruir_tabla_paginas`
 * para las tablas de nivel inferior.
 *
 * @param tabla_void Puntero a la tabla de páginas a destruir (se castea a `t_tabla_nivel*`).
 */
void destruir_tabla_paginas(void* tabla_void);

/**
 * @brief Maneja un fallo de página.
 *
 * Obtiene un marco libre (posiblemente desalojando otro), carga la página
 * desde SWAP si existe, o la inicializa con ceros si es nueva.
 * Actualiza la entrada de página para marcarla como presente y asignarle el marco.
 *
 * @param pid PID del proceso que sufrió el fallo de página.
 * @param entrada Puntero a la entrada de página que causó el fallo.
 * @param desplazamiento Desplazamiento dentro de la página.
 * @return Dirección física resultante del marco asignado más el desplazamiento,
 *         o -1 si no se pudo manejar el fallo (no hay marcos libres).
 */
int manejar_fallo_pagina(int pid, t_entrada_pagina* entrada);


//ESTO LO HACE CPU
/**
 * @brief Traduce una dirección lógica a una dirección física.
 *
 * Recorre las tablas de páginas del proceso (`pid`) nivel por nivel,
 * calculando los índices de entrada en cada tabla. Si se encuentra un fallo
 * de página, lo maneja.
 *
 * @param pid PID del proceso.
 * @param direccion_logica Dirección lógica a traducir.
 * @return Dirección física resultante, o -1 si hay un error (ej. tabla no encontrada, fallo de página no manejado).
 */
//int traducir_direccion(int pid, int direccion_logica, t_entrada_pagina** entrada_out);

void destruir_tabla_paginas(void *tabla_void);

int32_t obtener_marco_de_memoria(uint32_t numero_pagina, uint32_t* entradas_niveles, uint32_t pid);


#endif /* MEMORIA_TABLAS_H */