#ifndef CPU_H_
#define  CPU_H_

#include <pthread.h>

#include "cpu-conexiones.h"
#include "cpu-configs.h"
#include "cpu-log.h"
#include "ciclo-instruccion.h"

#include <utils/sockets.h>
#include <utils/hello.h>
#include <utils/configs.h>

/**
 * @brief Variable global para el PC
 */
extern uint32_t pc; // Variable global para el PC

#endif