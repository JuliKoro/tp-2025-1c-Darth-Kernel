#include "kernel.h"



int main(int argc, char* argv[]) {

    //Chequeo que los parametros sean correctos
    if(argc != 3) {
        fprintf(stderr, "Uso: %s [archivo_pseudocodigo] [tamanio_proceso]\n", argv[0]); 
        return EXIT_FAILURE;
    }

    //Obtengo los parametros
    int tamanio_proceso = atoi(argv[2]);
    char* archivo_pseudocodigo = argv[1];

    //Primero levanto las configs, despues el logger
    lista_io = list_create();
    inicializar_colas_y_sem();
    inicializar_configs();
    inicializar_logger_kernel();

    
    
    //Inicia el receptor de IO, esto recibe todos los IO y los agrega en la lista de IOs. Mantiene las cImplementarnexiones abiertas para que puedan recibir solicitudes.
    pthread_t thread_receptor_io;
    pthread_create(&thread_receptor_io, NULL, iniciar_receptor_io, NULL);
    pthread_detach(thread_receptor_io); 
    log_info(logger_kernel, "Receptor de IO iniciado");

    planificar_proceso_inicial(archivo_pseudocodigo, tamanio_proceso);


    //Inicia el planificador largo plazo
    printf("\t \n \n Presione ENTER para continuar con la planificación a largo plazo... \n \n");
    getchar();
    log_info(logger_kernel, "Planificador largo plazo iniciado en %s", kernel_configs.ingreasoaready);
    pthread_t thread_planificador_largo_plazo;
    pthread_create(&thread_planificador_largo_plazo, NULL, iniciar_planificador_largo_plazo, NULL);
    pthread_detach(thread_planificador_largo_plazo);




   
    // 3. Creamos dos PCBs de prueba.
        t_pcb* pcb_prueba_1 = malloc(sizeof(t_pcb));
        pcb_prueba_1->pid = 998;
        pcb_prueba_1->tamanio_proceso = 512;
        pcb_prueba_1->pc = 0;
        pcb_prueba_1->estado = RUNNING;

        t_pcb* pcb_prueba_2 = malloc(sizeof(t_pcb));
        pcb_prueba_2->pid = 999;
        pcb_prueba_2->tamanio_proceso = 512;
        pcb_prueba_2->pc = 0;
        pcb_prueba_2->estado = RUNNING;

        // 4. Agregamos ambos a la lista de ejecución.
        pthread_mutex_lock(&mutex_lista_executing);
        list_add(lista_executing, pcb_prueba_1);
        list_add(lista_executing, pcb_prueba_2);
        pthread_mutex_unlock(&mutex_lista_executing);
        log_info(logger_kernel, "[PRUEBA] PCBs de prueba (PID 998 y 999) creados y en estado RUNNING.");

        // 5. El primer proceso (998) pide la IO. Debería obtenerla inmediatamente.
        log_info(logger_kernel, "[PRUEBA] Enviando syscall IO para PID 998 (debería ser atendido)...");
        io("impresora", 3, 998); // Usará la impresora por 3 segundos.
        log_info(logger_kernel, "[PRUEBA] La llamada a io() para PID 998 ha sido realizada.");
        
        // 6. El segundo proceso (999) la pide inmediatamente después. Debería ser encolado.
        log_info(logger_kernel, "[PRUEBA] Enviando syscall IO para PID 999 (debería ser encolado)...");
        io("impresora", 2, 999); // Este proceso la quiere por 2 segundos.
        log_info(logger_kernel, "[PRUEBA] La llamada a io() para PID 999 ha sido realizada.");
        log_info(logger_kernel, "[PRUEBA] Observa los logs. El PID 999 debería ser atendido automáticamente cuando el 998 termine.");
    
    // --- FIN CODIGO DE PRUEBA DE IO - ESCENARIO 3 --- //

    

    // El kernel sigue su ejecución normal
    while(true){
        sleep(10);
        // printf("enviando syscall a io impresora de prueba\n");
        // io("impresora", 1000, 1);
        // printf("syscall enviada\n");
    }


    destruir_logger_kernel();
    config_destroy(kernel_tconfig);
    return 0;
}





// int enviar_solicitud_io(char* nombre_io, uint32_t pid, uint32_t tiempo){

//     pthread_mutex_lock(&mutex_io);
    
//     // Busco el IO en la lista
//     t_io* io = list_find_con_param(lista_io, comparar_nombre_io, nombre_io);
    
//     if(io == NULL) {
//         log_error(logger_kernel, "[IO-SOLICITUD] No se encontró el IO %s", nombre_io);
//         pthread_mutex_unlock(&mutex_io);
//         return;
//     }
    
//     // Busco una instancia disponible (pid_actual = -1)
//     t_instancia_io* instancia_disponible = NULL;
//     for(int i = 0; i < list_size(io->instancias_io); i++) {
//         t_instancia_io* instancia = list_get(io->instancias_io, i);
//         if(instancia->pid_actual == -1) {
//             instancia_disponible = instancia;
//             break;
//         }
//     }
    
//     if(instancia_disponible == NULL) {
//         log_error(logger_kernel, "[IO-SOLICITUD] No hay instancias disponibles para %s", nombre_io);
//         pthread_mutex_unlock(&mutex_io);
//         //TODO:Agregar PID a la cola de blocked del io

//         return;
//     }
    
//     // Marco la instancia como ocupada
//     instancia_disponible->pid_actual = pid;
//     io->instancias_disponibles--;
    
//     log_info(logger_kernel, "[IO-SOLICITUD] Enviando solicitud a %s (PID: %d, tiempo: %d)", nombre_io, pid, tiempo);
    
//     // Creo la solicitud
//     t_solicitud_io solicitud;
//     solicitud.pid = pid;
//     solicitud.tiempo = tiempo;
    
//     // Serializo y envío
//     t_buffer* buffer = serializar_solicitud_io(&solicitud);
//     t_paquete* paquete = empaquetar_buffer(PAQUETE_SOLICITUD_IO, buffer);
    
//     enviar_paquete(instancia_disponible->socket_io, paquete);

//     char* mensaje = recibir_mensaje(instancia_disponible->socket_io);
//     log_info(logger_kernel, "[IO-SOLICITUD] Mensaje recibido: %s", mensaje);
//     free(mensaje);
    
//     pthread_mutex_unlock(&mutex_io);
// }
