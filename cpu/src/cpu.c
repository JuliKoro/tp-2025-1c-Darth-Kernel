#include "cpu.h"

int main(int argc, char* argv[]) {
   
   // INICIO CPU
   // Verifco que se haya pasado el ID como argumento
   if (argc != 2) { 
      printf("Uso CPU: %s [identificador]\n", argv[0]);
      return EXIT_FAILURE;
   }

   // guardo el id del argumento
   int id_cpu = atoi(argv[1]);
   if (id_cpu <= 0) {
      fprintf(stderr, "El ID del CPU debe ser un número positivo.\n");
      return EXIT_FAILURE;
   }

   // CARGA CONFIGS
   // Se cargar la variable global 'cpu_configs'
   inicializar_configs();

   // INICIO LOGGERS
   t_log* logger_cpu = iniciar_logger_cpu(id_cpu); // crea logger cpu_<id>.log
   log_info(logger_cpu, "Iniciando CPU con ID: %d", id_cpu);

   // SOCKETS
   // Conexiones al Kernel (dispatch & interrupt)
   int socket_kernel_dispatch = cpu_conectar_a_kernel(cpu_configs.puertokerneldispatch, id_cpu);
   int socket_kernel_interrupt = cpu_conectar_a_kernel(cpu_configs.puertokernelinterrupt, id_cpu);

   // Conexion con Memoria
   int socket_memoria = cpu_conectar_a_memoria(id_cpu);
   
   while(1){ // LOOP CPU
      // ASIGANCION DE PROCESO

      // Espero y recibo desde Kernel (PID + PC)
      t_proceso* proceso = NULL;

      t_paquete* paquete = recibir_paquete(socket_kernel_dispatch);
      if(paquete == NULL) {
         log_error(logger_cpu, "Error al recibir paquete de Kernel. Cerrando conexion");
         return -1;
      }
      if(paquete->codigo_operacion == PAQUETE_INSTRUCCION_CPU) {
         log_info(logger_cpu, "Recibido paquete de instruccion de Kernel");
         proceso = deserializar_instruccion_cpu(paquete->buffer);
         log_info(logger_cpu, "PID: %d, PC: %d", proceso->pid, proceso->pc);
      }

      liberar_paquete(paquete);

      // LIMPIAR TLB Y CACHE DE PAGINAS

      //CICLO DE INSTRUCCION
      ciclo_instruccion(proceso, socket_memoria, socket_kernel_dispatch, socket_kernel_interrupt);
      
   }

   log_info(logger_cpu, "Finalizando ejecución del CPU %d", id_cpu);

   close(socket_memoria);
   log_destroy(logger_sockets);
   destruir_configs();

   return 0;
}

