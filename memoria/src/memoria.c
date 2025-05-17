#include "memoria.h"

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

// Retorna una const somulando que hay x cantidad de bytes disponibles en memoria
int obtener_espacio_libre_mock() {
    return 1024; // Retorna 1KB como espacio libre (solo de prueba, lo está simulando)
}

#define MAX_LINEA 256
#define MAX_INSTRUCCIONES 1024

// Define una estructura para almacenar las instrucciones de un proceso:
typedef struct {
    int pid;
    char** instrucciones;
    int cantidad_instrucciones;
} ProcesoInstrucciones;

ProcesoInstrucciones* cargar_instrucciones(const char* path_archivo, int pid) {
    FILE* archivo = fopen(path_archivo, "r");   // va a abrir un archivo pseudocódigo (uno por proceso en este caso)
    if (!archivo) {
        perror("Error al abrir archivo de pseudocódigo");
        return NULL;
    }

    ProcesoInstrucciones* proceso = malloc(sizeof(ProcesoInstrucciones));
    proceso->pid = pid;
    proceso->instrucciones = malloc(sizeof(char*) * MAX_INSTRUCCIONES);
    proceso->cantidad_instrucciones = 0;

    char buffer[MAX_LINEA];
    while (fgets(buffer, MAX_LINEA, archivo) != NULL) {
        // Elimina el salto de línea si lo hay
        buffer[strcspn(buffer, "\r\n")] = 0;

        proceso->instrucciones[proceso->cantidad_instrucciones] = strdup(buffer);
        proceso->cantidad_instrucciones++;
    }
    // Adentro de este while lo que está haciendo es leer linea por linea cada instrucción y 
    // se las afana (se las guarda, para quien no es de Lanús) en un array de stirngs

    fclose(archivo);
    return proceso; // devuelve un puntero a la estructura ProcesoInstrucciones
}

const char* obtener_instruccion(ProcesoInstrucciones* proceso, int pc) {
    if (pc < 0 || pc >= proceso->cantidad_instrucciones) {
        return NULL; // si está fuera de rango
    }
    return proceso->instrucciones[pc];
}
// Esta funcion te va a devolver la instruccion que corresponde al pc (program counter) que le pases


void liberar_proceso(ProcesoInstrucciones* proceso) {
    for (int i = 0; i < proceso->cantidad_instrucciones; i++) {
        free(proceso->instrucciones[i]); // chau memoria de cada instrucción (la liberamos)
    }
    free(proceso->instrucciones);
    free(proceso);  // lo mismo con el proceso completo
}

char* serializar_instrucciones(ProcesoInstrucciones* proceso) {
    size_t buffer_size = MAX_LINEA * proceso->cantidad_instrucciones;
    char* buffer = malloc(buffer_size);
    buffer[0] = '\0'; // Inicializa el buffer como string vacío

    for (int i = 0; i < proceso->cantidad_instrucciones; i++) {
        strcat(buffer, proceso->instrucciones[i]);
        if (i < proceso->cantidad_instrucciones - 1) {
            strcat(buffer, "\n"); // Agrega un salto de línea entre instrucciones
        }
    }
    // en este for lo que hace es concatenar cada instrucción al buffer (que es un string)
    // es string se puede enviar por socket ewe

    return buffer;
}

void enviar_instrucciones(int socket, ProcesoInstrucciones* proceso) {
    char* paquete = serializar_instrucciones(proceso); 
    enviar_mensaje(paquete, socket);
    free(paquete);
}
// Llama a la funcion de serialización y lo envía por socket con la funcion enviar_mensaje

int main(int argc, char* argv[]) {

    const char* path = "/home/utnso/scripts/1234"; // esto despues hay que cambiarlo. 
    // Tiene que ser un pseudocódigo por cada proceso dentro de la ruta que pide el enunciado (en /untso/scripts)
    int pid = 1; // Es un ejemplo

    saludar("memoria");
    inicializar_configs();

    int socket_memoria = iniciar_servidor_memoria();

    enviar_mensaje("Hola como andas :3 soy la memoria", socket_memoria);

    char* mensaje = recibir_mensaje(socket_memoria);
    log_info(logger_sockets, "Me llego esto: %s", mensaje);

    log_info(logger_sockets, "Voy a cargar las instrucciones");
    ProcesoInstrucciones* proceso = cargar_instrucciones(path, pid);
    if (!proceso) {
        return 1;   // aca pincha si no podemos cargar el archivo
    }
    // Acá usar cargar_instrucciones para leer el archivo y justamente, obtener esas instrucciones

    
    for (int pc = 0; pc < proceso->cantidad_instrucciones; pc++) {
        const char* instruccion = obtener_instruccion(proceso, pc);
        printf("## PID: %d - Obtener instrucción: %d - Instrucción: %s\n", pid, pc, instruccion);
    }

    log_info(logger_sockets, "Voy a obtener espacio libre mock");
    obtener_espacio_libre_mock();
    int resultado = obtener_espacio_libre_mock();
    log_info(logger_sockets, "Espacio libre mock obtenido correctamente: %i", resultado);
    // muestra el dato de color de lo que vale el mock nada mas

    log_info(logger_sockets, "Enviando instrucciones al otro módulo...");
    enviar_instrucciones(socket_memoria, proceso);    

    liberar_proceso(proceso);
    free(mensaje);

    log_destroy(logger_sockets);
    config_destroy(memoria_tconfig);

    return 0;
}


