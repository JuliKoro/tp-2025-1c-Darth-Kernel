#include "ciclo-instruccion.h"

void ciclo_instruccion(int pid, int pc, int socket_memoria){
    while(1){ // loop ciclo de instruccion
        
        fetch(pid, pc, socket_memoria);
        decode();
        check_interrupt();
    }
    
}

void fetch(int pid, int pc, int socket_memoria){
    //Le pido a memoria la instruccion
    t_paquete* paquete = crear_paquete();
    agregar_a_paquete(paquete, pid);
    agregar_a_paquete(paquete, pc);
    enviar_paquete(paquete, socket_memoria);
    free(paquete);
    
    // Recibe la instruccion de Memoria :)
    char* instruccion = recibir_mensaje(socket_memoria);
    printf("Instruccion recibida:\n%s\n", instruccion);

   return instruccion;
}