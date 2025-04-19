#ifndef UTILS_CONFIGS_H_
#define UTILS_CONFIGS_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>

/**
* @brief Carga el archivo config del modulo
* @param path el path del archivo
* @return devuelve un t_config* nuevo
*/

t_config* inciar_config (char* path);

#endif
