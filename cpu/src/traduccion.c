#include <stdint.h>

#include "traduccion.h"

t_direccion_fisica direccion_logica_a_fisica(uint32_t direccion_logica){
    t_direccion_fisica direccion_fisica;
    //Obtengo los valores de la dirección física
    direccion_fisica.nro_pagina = floor(direccion_logica / cpu_configs.tampagina); //El tamaño de página lo agregué a las configs
    //direccion_fisica.entrada_nivel_X = floor(direccion_fisica.nro_pagina  / cpu_configs.entradastlb ^ (N - X)) % cpu_configs.entradastlb
    direccion_fisica.desplazamiento = direccion_logica % cpu_configs.tampagina;
    return direccion_fisica;
}