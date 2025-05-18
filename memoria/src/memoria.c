#include "memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>

// No olividar de incluir el log obligatorio: Conexión de Kernel: “## Kernel Conectado - FD del socket: <FD_DEL_SOCKET>” 

/*
Check 2:
    - Leer un archivo del directorio PATH_INSTRUCCIONES el cual va a tener las instrucciones del proceso
    - Asociarlas a su PID para que estén disponibles cunado CPU las pida
    - Tenerlas cargadas en mmemoria (puede ser una estructura tipo map<PID, lista de instrucciones>)
*/

/*
    El dia de mañana que se implemente el PCB se hace uso del Program Counter (PC) para saber que instrucción ejecutar
    y asi mismo no repetir archivos de instrucciones.
*/

// Define una estructura para almacenar las instrucciones de un proceso:
typedef struct {
    int pid;
    char** instrucciones;
    int cantidad_instrucciones;
} T_Proceso;

// Globales
t_dictionary* procesos_en_memoria;
t_log* logger_memoria;

// Retorna una const somulando que hay x cantidad de bytes disponibles en memoria
int obtener_espacio_libre_mock() {
    return 1024; // Retorna 1KB como espacio libre (solo de prueba, lo está simulando)
}

T_Proceso* leer_archivo_de_proceso(const char* filepath, int pid) {
    FILE* archivo = fopen(filepath, "r");
    if (!archivo) {
        log_error(logger_memoria, "No se pudo abrir el archivo del PID %d", pid);
        return NULL;
    }

    char** instrucciones = malloc(sizeof(char*) * 100); // Se puede ajustar dinámicamente luego
    int cantidad = 0;

    char linea[256];
    while (fgets(linea, sizeof(linea), archivo)) {
        linea[strcspn(linea, "\n")] = 0; // Remueve el salto de línea
        instrucciones[cantidad] = strdup(linea); // Copia segura
        cantidad++;
    }

    fclose(archivo);

    T_Proceso* proceso = malloc(sizeof(T_Proceso));
    proceso->pid = pid;
    proceso->instrucciones = instrucciones;
    proceso->cantidad_instrucciones = cantidad;

    return proceso;
}

void destruir_proceso(void* proceso_void) {
    T_Proceso* proceso = (T_Proceso*) proceso_void;
    for (int i = 0; i < proceso->cantidad_instrucciones; i++) {
        free(proceso->instrucciones[i]);
    }
    
    free(proceso->instrucciones);
    free(proceso);

    log_info(logger_memoria, "## PID: %d - Proceso Destruido - ...", proceso->pid);
    /*
    ESTE ES EL LOGGER OBLIGATORIO, POR AHORA (check 2) HAY INFO QUE TODAVÍA NO TENGO, POR LO QUE SOLO VOY A AVISAR DE LA DESTRUCCION DEL PROCESO CON SU RESPECTIVO PID
    Destrucción de Proceso: “## PID: <PID> - Proceso Destruido - Métricas - Acc.T.Pag: <ATP>; Inst.Sol.: <Inst.Sol.>; 
    SWAP: <SWAP>; Mem.Prin.: <Mem.Prin.>; Lec.Mem.: <Lec.Mem.>; Esc.Mem.: <Esc.Mem.>”
     */
}

void cargar_proceso(int pid, const char* nombre_archivo) {

    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", memoria_configs.pathinstrucciones, nombre_archivo);

    T_Proceso* proceso = leer_archivo_de_proceso(fullpath, pid);
    if (proceso != NULL) {
        dictionary_put(procesos_en_memoria, strdup(nombre_archivo), proceso);
    }

    // Creación de Proceso: “## PID: <PID> - Proceso Creado - Tamaño: <TAMAÑO>”
    log_info(logger_memoria, "## PID %d - Proceso Creado - Tamaño: <TAMAÑO>", pid);

}

void cargar_procesos_en_memoria() {
    //procesos_en_memoria = dictionary_create();
    if (procesos_en_memoria != NULL) {
        dictionary_destroy_and_destroy_elements(procesos_en_memoria, (void*) destruir_proceso);
    }
    procesos_en_memoria = dictionary_create();

    DIR* dir = opendir(memoria_configs.pathinstrucciones);
    if (!dir) {
        log_error(logger_memoria, "No se pudo abrir el directorio de instrucciones");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Solo archivos regulares
            log_info(logger_memoria, "Leyendo archivo: %s", entry->d_name);
            int pid = atoi(entry->d_name); // Convierte nombre de archivo a PID
            if (pid == 0 && strcmp(entry->d_name, "0") != 0)
                continue; // No es un número válido

            cargar_proceso(pid, entry->d_name);
 
        }
    }

    closedir(dir);
}

    // Ejemplo: imprimir todos los procesos cargados
    void mostrar_proceso(char* key, void* value) {
        T_Proceso* proceso = (T_Proceso*)value;
        printf("PID: %d\n", proceso->pid);
        for (int i = 0; i < proceso->cantidad_instrucciones; i++) {
            printf("  - %s\n", proceso->instrucciones[i]);
        }
    }


/*
Obtener instrucción: “## PID: <PID> - Obtener instrucción: <PC> - Instrucción: <INSTRUCCIÓN> <...ARGS>”
*/

int main(int argc, char* argv[]) {

    logger_memoria = log_create("memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);


    //const char* path = "/home/utnso/scripts/1234"; // esto despues hay que cambiarlo. 
    // Tiene que ser un pseudocódigo por cada proceso dentro de la ruta que pide el enunciado (en /untso/scripts)
    

    inicializar_configs();
    int socket_memoria = iniciar_servidor_memoria();
    

    log_info(logger_memoria, "Iniciando módulo Memoria...");
    cargar_procesos_en_memoria();


    dictionary_iterator(procesos_en_memoria, mostrar_proceso);


/*
    log_info(logger_sockets, "Voy a obtener espacio libre mock");
    obtener_espacio_libre_mock();
    int resultado = obtener_espacio_libre_mock();
    log_info(logger_sockets, "Espacio libre mock obtenido correctamente: %i", resultado);
    // muestra el dato de color de lo que vale el mock nada mas
*/

    dictionary_destroy_and_destroy_elements(procesos_en_memoria, free); // Si querés liberar también instrucciones hay que hacer free de cada string
    log_destroy(logger_sockets);
    log_destroy(logger_memoria);
    config_destroy(memoria_tconfig);

    return 0;
}


