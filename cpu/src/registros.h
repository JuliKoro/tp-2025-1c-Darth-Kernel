#ifndef REGISTROS_H_
#define REGISTROS_H_

#include <stdint.h>

/**
 * @brief Variable global para el PC (Program Counter)
 * @note Si se usan hilos, hay que proteger la variable con mutex
 */
extern uint32_t PC; // Variable global para el PC (Program Counter)

/**
 * @brief Variable global para el IF (Interrupt Flag)
 * @note Si se usan hilos, hay que proteger la variable con mutex
 */
extern bool IF; // Variable global que representa el registro IF (Interrupt Flag)

/**
 * @brief Variable global que representa el registro TSC (Time Stamp Counter)
 * @note Es un registro especial que lleva la cuenta de la cantidad de ciclos de reloj que han transcurrido desde que se encendió el procesador. El TSC no se reinicia entre procesos ni hilos, y no mide tiempo en segundos, sino ciclos de CPU.
 */
extern uint32_t TSC;

#endif