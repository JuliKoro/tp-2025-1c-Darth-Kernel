#ifndef MEMORIA_OPERACIONES_H
#define MEMORIA_OPERACIONES_H

#include "memoria.h"

/**
 * @brief Cabecera para las operaciones de lectura/escritura en memoria.
 *
 * Declara las funciones para realizar operaciones de lectura y escritura
 * en la memoria principal, tanto a nivel de bytes como de páginas completas.
 */

/**
 * @brief Aplica un retardo basado en `RETARDO_MEMORIA`.
 *
 * Utiliza `usleep` para pausar la ejecución por el tiempo configurado
 * en milisegundos.
 */
void aplicar_retardo_memoria();

/**
 * @brief Lee datos de la memoria principal.
 *
 * Traduce la dirección lógica a física, valida los límites y copia `tam` bytes
 * desde la memoria principal a un nuevo buffer.
 *
 * @param pid PID del proceso.
 * @param direccion_fisica Dirección fisica de inicio de la lectura.
 * @param tam Cantidad de bytes a leer.
 * @return Puntero a un nuevo buffer con los datos leídos, or NULL si hay un error.
 */
void* leer_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tam);

/**
 * @brief Escribe datos en la memoria principal.
 *
 * Traduce la dirección lógica a física, valida los límites y copia `tam` bytes
 * desde `valor` a la memoria principal. Marca la página como modificada.
 *
 * @param pid PID del proceso.
 * @param direccion_logica Dirección lógica de inicio de la escritura.
 * @param tam Cantidad de bytes a escribir.
 * @param valor Puntero a los datos a escribir.
 * @return true si la escritura fue exitosa, false en caso contrario.
 */
bool escribir_memoria(int pid, int direccion_logica, int tam, void* valor);

/**
 * @brief Lee el contenido completo de una página en memoria principal.
 *
 * Copia el contenido de una página completa (definida por `tampagina`)
 * desde la `direccion_fisica` (que debe ser el inicio de una página)
 * a un nuevo buffer.
 *
 * @param pid PID del proceso (para logging).
 * @param direccion_fisica Dirección física de inicio de la página.
 * @return Puntero a un nuevo buffer con el contenido de la página, o NULL si hay un error.
 */
void* leer_pagina_completa(int pid, int direccion_fisica);

/**
 * @brief Actualiza el contenido completo de una página en memoria principal.
 *
 * Copia el `contenido` proporcionado a la `direccion_fisica` (que debe ser
 * el inicio de una página) en la memoria principal. Marca la página como modificada.
 *
 * @param pid PID del proceso (para logging).
 * @param direccion_fisica Dirección física de inicio de la página.
 * @param contenido Puntero al buffer con los nuevos datos de la página.
 * @return true si la actualización fue exitosa, false en caso contrario.
 */
bool actualizar_pagina_completa(int pid, int direccion_fisica, void* contenido);

#endif /* MEMORIA_OPERACIONES_H */