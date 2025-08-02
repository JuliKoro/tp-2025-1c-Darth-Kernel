#include "memoria.h"
#include "memoria-operaciones.h"
#include "memoria-admin.h"
#include "memoria-procesos.h"
#include "memoria-tablas.h"

#include <unistd.h>  // usleep
#include <string.h>
#include <stdlib.h>

static void marcar_modificado_recursivo(t_tabla_nivel *tabla, int pid, int numero_marco_fisico);


 /**
  * @brief Aplica un retardo basado en `RETARDO_MEMORIA`.
  *
  * Utiliza `usleep` para pausar la ejecución por el tiempo configurado
  * en milisegundos.
  */
 void aplicar_retardo_memoria() {
    if (memoria_configs.retardomemoria > 0) {
        usleep(memoria_configs.retardomemoria * 1000); // Convertir ms a microsegundos
    }
}

/**
  * @brief Lee datos de la memoria principal.
  *
  * Recibe la dirección física y copia `tam` bytes
  * desde la memoria principal a un nuevo buffer.
  *
  * @param pid PID del proceso.
  * @param direccion_fisica Dirección fisica de inicio de la lectura.
  * @param tam Cantidad de bytes a leer.
  * @return Puntero a un nuevo buffer con los datos leídos, o NULL si hay un error.
  */

 void* leer_memoria(uint32_t pid, uint32_t direccion_fisica, uint32_t tam) {

    void *buffer = malloc(tam);
    if (!buffer) return NULL;
    memcpy(buffer, (char *)administrador_memoria->memoria_principal + direccion_fisica, tam);
    actualizar_metricas(pid, LECTURA_MEMORIA_MET); // Métrica de lectura de memoria

    log_info(logger_memoria, "## PID: %d - Lectura - Dir. Física: %d - Tamaño: %d", pid, direccion_fisica, tam); //LOG OBLIGATORIO

    aplicar_retardo_memoria(); // Aplicar retardo por acceso a memoria
    return buffer;
}

 /**
  * @brief Escribe datos en la memoria principal.
  *
  * Marca la página como modificada.
  *
  * @param pid PID del proceso.
  * @param direccion_fisica Dirección fisica de inicio de la escritura.
  * @param tam Cantidad de bytes a escribir.
  * @param valor Puntero a los datos a escribir.
  * @return true si la escritura fue exitosa, false en caso contrario.
  */
 bool escribir_memoria(int pid, int direccion_fisica, int tam, void* valor) {
    // Validar límites de la dirección física
    if (direccion_fisica < 0 || direccion_fisica + tam > memoria_configs.tammemoria) {
        log_error(logger_memoria, "PID %d: Intento de escritura fuera de los límites. Dir. Física: %d, Tamaño: %d.", pid, direccion_fisica, tam);
        return false;
    }

    // Validar que la memoria principal esté inicializada
    if (!administrador_memoria->memoria_principal) {
        log_error(logger_memoria, "PID %d: Memoria principal no inicializada.", pid);
        return false;
    }

    // Escribir datos en memoria principal
    memcpy((char*)administrador_memoria->memoria_principal + direccion_fisica, valor, tam);

    // Calcular el número de marco físico
    int numero_marco_fisico = direccion_fisica / memoria_configs.tampagina;

    // Buscar la tabla de páginas del proceso y marcar la página como modificada
    char pid_str[16];
    sprintf(pid_str, "%d", pid);
    t_tabla_nivel* tabla_nivel_0 = dictionary_get(administrador_memoria->tablas_paginas, pid_str);

    if (tabla_nivel_0) {
        marcar_modificado_recursivo(tabla_nivel_0, pid, numero_marco_fisico);
    } else {
        log_warning(logger_memoria, "PID %d: No se encontró tabla de páginas para marcar marco %d como modificado.", pid, numero_marco_fisico);
    }

    // Actualizar métricas y logs
    actualizar_metricas(pid, ESCRITURA_MEMORIA_MET);
    log_info(logger_memoria, "## PID: %d - Escritura - Dir. Física: %d - Tamaño: %d", pid, direccion_fisica, tam);
    aplicar_retardo_memoria(); // Aplicar retardo por acceso a memoria

    return true;
}



/* 
  bool escribir_memoria(int pid, int direccion_fisica, int tam, void* valor) {

    memcpy((char*)administrador_memoria->memoria_principal + direccion_fisica, valor, tam);

    actualizar_metricas(pid, ESCRITURA_MEMORIA_MET);
    log_info(logger_memoria, "## PID: %d - Escritura - Dir. Física: %d - Tamaño: %d", pid, direccion_fisica, tam);
    aplicar_retardo_memoria();
    return true;
}
*/
  /*
 bool escribir_memoria(int pid, int direccion_logica, int tam, void* valor) {
    char pid_str[16];

    t_entrada_pagina* entrada = NULL;
    int direccion_fisica = traducir_direccion(pid, direccion_logica, &entrada);
    
    if (direccion_fisica < 0) {
        log_error(logger_memoria, "PID %d: Fallo al escribir memoria. Error al traducir dirección lógica %d.", pid, direccion_logica);
        return false;
    }

    // Validar que la escritura no exceda los límites de la memoria principal
    if (direccion_fisica + tam > memoria_configs.tammemoria || direccion_fisica < 0) {
        log_error(logger_memoria, "PID %d: Intento de escritura fuera de los límites de la memoria principal. Dir. Física: %d, Tamaño: %d.", pid, direccion_fisica, tam);
        return false;
    }

    memcpy((char*)administrador_memoria->memoria_principal + direccion_fisica, valor, tam);

    // Marcar la página como modificada
    // Para esto, necesitamos encontrar la entrada de página que corresponde a esta dirección física.
    // Esto implica recorrer las tablas de páginas para el PID dado.
    sprintf(pid_str, "%d", pid);
    t_tabla_nivel* tabla_nivel_0 = dictionary_get(administrador_memoria->tablas_paginas, pid_str);

    if (tabla_nivel_0) {
        // Calcular el número de marco físico donde se realizó la escritura
        int numero_marco_fisico = direccion_fisica / memoria_configs.tampagina;

        // Función auxiliar recursiva para encontrar y marcar la entrada de página
        void _marcar_modificado_recursivo(t_tabla_nivel* current_tabla) {
            for (int i = 0; i < memoria_configs.entradasportabla; i++) {
                t_entrada_pagina* entrada = current_tabla->entradas[i];

                if (current_tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
                    // Si es una entrada de tabla de nivel intermedio, ir al siguiente nivel
                    if (entrada->presente) {
                        t_tabla_nivel* next_tabla = (t_tabla_nivel*)(intptr_t)entrada->marco;
                        if (next_tabla) {
                            _marcar_modificado_recursivo(next_tabla);
                        }
                    }
                } else {
                    // Último nivel: es una página de datos
                    if (entrada->presente && entrada->marco == numero_marco_fisico) {
                        entrada->modificado = true;
                        log_debug(logger_memoria, "PID %d: Página en marco %d marcada como modificada.", pid, numero_marco_fisico);
                        return; // Encontrado y marcado, salir
                    }
                }
            }
        }
        _marcar_modificado_recursivo(tabla_nivel_0);
    } else {
        log_warning(logger_memoria, "PID %d: No se encontró la tabla de páginas para marcar la página como modificada después de la escritura.", pid);
    }

    actualizar_metricas(pid, ESCRITURA_MEMORIA_MET); // Métrica de escritura de memoria
    log_info(logger_memoria, "## PID: %d - Escritura - Dir. Física: %d - Tamaño: %d", pid, direccion_fisica, tam);
    aplicar_retardo_memoria(); // Aplicar retardo por acceso a memoria
    return true;
}
    */

    
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
 /*
 void* leer_pagina_completa(int pid, int direccion_fisica) {
    if (direccion_fisica % memoria_configs.tampagina != 0) {
        log_error(logger_memoria, "PID %d: Dirección física %d no es el inicio de una página. No se puede leer página completa.", pid, direccion_fisica);
        return NULL;
    }
    if (direccion_fisica < 0 || direccion_fisica + memoria_configs.tampagina > memoria_configs.tammemoria) {
        log_error(logger_memoria, "PID %d: Dirección física %d fuera de los límites de la memoria principal. No se puede leer página completa.", pid, direccion_fisica);
        return NULL;
    }

    void* contenido_pagina = malloc(memoria_configs.tampagina);
    if (contenido_pagina == NULL) {
        log_error(logger_memoria, "PID %d: Error al asignar memoria para el contenido de la página completa.", pid);
        return NULL;
    }
    memcpy(contenido_pagina, (char*)administrador_memoria->memoria_principal + direccion_fisica, memoria_configs.tampagina);
    log_info(logger_memoria, "## PID: %d - Lectura de página completa - Dir. Física: %d", pid, direccion_fisica);
    aplicar_retardo_memoria(); // Aplicar retardo por acceso a memoria
    return contenido_pagina;
}
*/

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

  static void marcar_modificado_recursivo(t_tabla_nivel *current_tabla, int pid, int numero_marco_fisico) {
    for (int i = 0; i < memoria_configs.entradasportabla; i++) {
        t_entrada_pagina *entrada = current_tabla->entradas[i];
        if (!entrada) continue;
        if (current_tabla->nivel_actual < memoria_configs.cantidadniveles - 1) {
            if (entrada->presente) {
                t_tabla_nivel *next = entrada->subnivel;
                marcar_modificado_recursivo(next, pid, numero_marco_fisico);
            }
        } else {
            if (entrada->presente && entrada->marco == numero_marco_fisico) {
                entrada->modificado = true;
                log_debug(logger_memoria, "PID %d: Página marco %d marcada modificada", pid, numero_marco_fisico);
                return;
            }
        }
    }
}

/*

 bool actualizar_pagina_completa(int pid, int direccion_fisica, void* contenido) {
    if (direccion_fisica % memoria_configs.tampagina != 0) {
        log_error(logger_memoria, "PID %d: Dirección física %d no es el inicio de una página. No se puede actualizar página completa.", pid, direccion_fisica);
        return false;
    }
    if (direccion_fisica < 0 || direccion_fisica + memoria_configs.tampagina > memoria_configs.tammemoria) {
        log_error(logger_memoria, "PID %d: Dirección física %d fuera de los límites de la memoria principal. No se puede actualizar página completa.", pid, direccion_fisica);
        return false;
    }

    memcpy((char*)administrador_memoria->memoria_principal + direccion_fisica, contenido, memoria_configs.tampagina);

    // Marcar la página como modificada
    int numero_marco_fisico = direccion_fisica / memoria_configs.tampagina;
    char pid_key[16];
    sprintf(pid_key, "%d", pid);

    t_tabla_nivel *tabla_nivel_0 = dictionary_get(administrador_memoria->tablas_paginas, pid_key);
    
    if (tabla_nivel_0) {
        marcar_modificado_recursivo(tabla_nivel_0, pid, numero_marco_fisico);
    }

    log_info(logger_memoria, "## PID: %d - Actualización de página completa - Dir. Física: %d", pid, direccion_fisica);
    aplicar_retardo_memoria(); // Aplicar retardo por acceso a memoria
    return true;
}
    */