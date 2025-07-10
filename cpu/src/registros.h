#ifndef REGISTROS_H_
#define REGISTROS_H_

#include <stdint.h>

/**
 * @brief Variable global para el PC
 * @note Si se usan hilos, hay que proteger la variable con mutex
 */
extern uint32_t pc; // Variable global para el PC

#endif