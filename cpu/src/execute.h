#ifndef EXECUTE_H_
#define EXECUTE_H_

#include <stdint.h>

#include <commons/log.h>

#include "ciclo-instruccion.h"
#include "cpu-conexiones.h"

/**
 * @brief Ejecuta una instrucción decodificada del CPU
 * 
 * Esta función toma una instrucción decodificada y la ejecuta.
 * Dependiendo del tipo de instrucción, puede realizar diferentes
 * acciones, como leer o escribir en memoria, realizar un salto,
 * o manejar operaciones de entrada/salida.
 * 
 * @param instruccion Puntero a la estructura de instrucción decodificada
 *                    que se desea ejecutar.
 * @param socket_memoria Socket de memoria para realizar operaciones
 *                       de lectura y escritura.
 * @return int Retorna 0 si la ejecución fue exitosa, o un código de error
 *             en caso de fallo.
 */
int execute(instruccion_decodificada* instruccion, int socket_memoria);

#endif 
