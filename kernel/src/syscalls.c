#include "syscalls.h"


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Funciones auxiliares para las syscalls

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

t_io* buscar_io_por_nombre(char* nombre_buscado) {
    pthread_mutex_lock(&mutex_io);

    for(int i = 0; i < list_size(lista_io); i++) {
        t_io* io = list_get(lista_io, i);
        if(strcmp(io->nombre_io, nombre_buscado) == 0) {
            pthread_mutex_unlock(&mutex_io);
            return io;
        }
    }
    pthread_mutex_unlock(&mutex_io);
    return NULL;
}



/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                Syscalls como tal

/////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


void init_proc(char* archivo_pseudocodigo, int tamanio_proceso) {
    t_pcb* pcb = inicializar_pcb(obtener_pid_siguiente(), archivo_pseudocodigo, tamanio_proceso);
    agregar_pcb_a_cola_new(pcb);
} 

void io (char* nombre_io, int tiempo_io) {
    //Buscar IO en la lista de IO
    t_io* io = buscar_io_por_nombre(nombre_io);
    //Si no existe, proceso va a EXIT
    if(io == NULL) {
        //TODO: enviar a exit
    }
    //Si existe, proceso va a BLOCKED

    //Se envia la peticion al IO
    //pthread_mutex_lock(&io->mutex_cola_blocked_io);
    //queue_push(io->cola_blocked_io, obtener_pid_actual()); // Esto esta mal
    pthread_mutex_unlock(&io->mutex_cola_blocked_io);
    //Se espera a que el IO finalice la operacion

    //Se actualiza la cola de ready

    //Se actualiza la cola de blocked

    //Se actualiza la cola de executing

    //Se actualiza la cola de exit

    //Se actualiza la cola de new   
}