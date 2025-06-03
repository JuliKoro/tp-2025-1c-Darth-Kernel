#include "memoria.h"


// No olividar de incluir el log obligatorio: Conexión de Kernel: "## Kernel Conectado - FD del socket: <FD_DEL_SOCKET>" 

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

// Globales


int main(int argc, char* argv[]) {

    //const char* path = "/home/utnso/scripts/1234"; // esto despues hay que cambiarlo. 
    // Tiene que ser un pseudocódigo por cada proceso dentro de la ruta que pide el enunciado (en /untso/scripts)
    
    inicializar_configs();
    inicializar_logger_memoria();


    procesos_en_memoria = dictionary_create();  // Inicializamos antes del servidor
    int socket_memoria = iniciar_servidor_memoria();
    log_info(logger_memoria, "Iniciando módulo Memoria...");
    
    cargar_procesos_en_memoria();
    dictionary_iterator(procesos_en_memoria, mostrar_proceso);
    obtener_instruccion(1234, 1); // Estas dos lineas están simulando el pedido de una instrucción por parte de CPU
    obtener_instruccion(1234, 4);



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


