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

extern bool exit_flag;

#endif