#include "instrucciones.h"

t_dictionary* procesos_en_memoria;

int obtener_espacio_libre_mock() {
    return 1024; // Retorna 1KB como espacio libre (solo de prueba, lo está simulando)
}

t_proceso* leer_archivo_de_proceso(const char* filepath, int pid) {
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

    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->pid = pid;
    proceso->instrucciones = instrucciones;
    proceso->cantidad_instrucciones = cantidad;

    return proceso;
}

void destruir_proceso(void* proceso_void) {
    t_proceso* proceso = (t_proceso*) proceso_void;
    for (int i = 0; i < proceso->cantidad_instrucciones; i++) {
        free(proceso->instrucciones[i]);
    }
    
    log_info(logger_memoria, "## PID: %d - Proceso Destruido - ...", proceso->pid);

    free(proceso->instrucciones);
    free(proceso);

    
    /*
    ESTE ES EL LOGGER OBLIGATORIO, POR AHORA (check 2) HAY INFO QUE TODAVÍA NO TENGO, POR LO QUE SOLO VOY A AVISAR DE LA DESTRUCCION DEL PROCESO CON SU RESPECTIVO PID
    Destrucción de Proceso: "## PID: <PID> - Proceso Destruido - Métricas - Acc.T.Pag: <ATP>; Inst.Sol.: <Inst.Sol.>; 
    SWAP: <SWAP>; Mem.Prin.: <Mem.Prin.>; Lec.Mem.: <Lec.Mem.>; Esc.Mem.: <Esc.Mem.>"
     */
}

void cargar_proceso(int pid, const char* nombre_archivo) {
    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", memoria_configs.pathinstrucciones, nombre_archivo);

    t_proceso* proceso = leer_archivo_de_proceso(fullpath, pid);
    if (proceso != NULL) {
        char* pid_key = malloc(10);
        sprintf(pid_key, "%d", pid);
        dictionary_put(procesos_en_memoria, pid_key, proceso);
        log_info(logger_memoria, "## PID %d - Proceso Creado - Tamaño: <TAMAÑO>", pid);
    }
}

void cargar_procesos_en_memoria() {
    if (procesos_en_memoria == NULL) {
        procesos_en_memoria = dictionary_create();
    }


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

void mostrar_proceso(char* key, void* value) {
    t_proceso* proceso = (t_proceso*)value;
    printf("PID: %d\n", proceso->pid);
    for (int i = 0; i < proceso->cantidad_instrucciones; i++) {
        printf("  - %s\n", proceso->instrucciones[i]);
    }
}

const char* obtener_instruccion(int pid, int pc) {
    char pid_key[10];
    sprintf(pid_key, "%d", pid);

    t_proceso* proceso = dictionary_get(procesos_en_memoria, pid_key);
    if (proceso != NULL && pc <= proceso->cantidad_instrucciones) {
        const char* instruccion = proceso->instrucciones[pc-1];
        log_info(logger_memoria, "## PID %d - Obtener instrucción: %d - Instrucción: %s", pid, pc, instruccion);
        return instruccion;
    }

    //log_info(logger_memoria, "No se encontró el proceso con PID %d o PC fuera de rango", pid);
    log_info(logger_memoria, ":)) Disculpe, me puede mandar una patrulla acá a la colonia Morelos, 5º sector, pero desde ya porque va a dar chingazos.");

    return NULL;
}