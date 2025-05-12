#ifndef UTILS_CONFIGS_H_
#define UTILS_CONFIGS_H_

//considerar renombrar este archivo para no confundir con config.h

#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>

/**
* @brief Carga el archivo config del modulo
* @param path el path del archivo
* @return devuelve un t_config* nuevo
*/

t_config* iniciar_config (char* path);

int cargar_variable_int(t_config* config, char* nombre);

char* cargar_variable_string(t_config* config, char* nombre);

double cargar_variable_double(t_config* config, char* nombre);

#endif
