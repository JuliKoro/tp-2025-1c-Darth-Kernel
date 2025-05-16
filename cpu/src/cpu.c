#include "cpu.h"

int main(int argc, char* argv[]) {
   

   inicializar_configs();

   //cpu_conectar_a_memoria();


   int socket_memoria = cpu_conectar_a_memoria();

   enviar_mensaje("Hola memoria como andas :3 soy el CPU", socket_memoria);

   char* mensaje = recibir_mensaje(socket_memoria);

   log_info(logger_sockets, "Me llego esto: %s", mensaje);
   
   // Aca reciben las instrucciones de Memoria :)
   char* paquete = recibir_mensaje(socket_memoria);
   printf("Instrucciones recibidas:\n%s\n", paquete);
   free(paquete);

   free(mensaje);
   close(socket_memoria);
   log_destroy(logger_sockets);
   config_destroy(cpu_tconfig);

   return 0;
}
