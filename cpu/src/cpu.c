#include "cpu.h"

int main(int argc, char* argv[]) {
   

   inicializar_configs();

   //cpu_conectar_a_memoria();


   int socket_memoria = cpu_conectar_a_memoria();

   enviar_mensaje("Hola memoria como andas :3 soy el CPU", socket_memoria);

   char* mensaje = recibir_mensaje(socket_memoria);

   log_info(logger_sockets, "Me llego esto: %s", mensaje);

   free(mensaje);
   close(socket_memoria);
   log_destroy(logger_sockets);
   config_destroy(cpu_tconfig);

   return 0;
}
