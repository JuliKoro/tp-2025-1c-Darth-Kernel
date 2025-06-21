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
      t_list* paquete_proceso = recibir_paquete(socket_kernel_dispatch);
      uint32_t* pid = list_get (paquete_proceso, 1);
      uint32_t* pc = list_get (paquete_proceso, 2);
      printf("Proceso recibido:\n");
      log_info(logger_cpu, "## PID: %ls - FETCH - Program Counter: %ls", pid, pc);

      // LIMPIAR TLB Y CACHE DE PAGINAS

      //CICLO DE INSTRUCCION
      ciclo_instruccion(pid, pc, socket_memoria);
      
   }

   log_info(logger_cpu, "Finalizando ejecución del CPU %d", id_cpu);

   close(socket_memoria);
   log_destroy(logger_sockets);
   destruir_configs();

   return 0;
}

