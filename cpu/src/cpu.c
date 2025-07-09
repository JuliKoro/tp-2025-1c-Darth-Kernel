#include "cpu.h"

int socket_kernel_dispatch;
int socket_kernel_interrupt;
int socket_memoria;
t_log* logger_cpu;

// Mutex para proteger el acceso a la variable proceso
pthread_mutex_t mutex_proceso = PTHREAD_MUTEX_INITIALIZER; // Inicialización estática

t_proceso_cpu* proceso = NULL;

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
   // Se carga la variable global 'cpu_configs'
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

   // HILOS
   pthread_t thread_dispatch, thread_interrupt, thread_ciclo_instruccion;

   // Crear hilo para recibir instrucciones del kernel
   pthread_create(&thread_dispatch, NULL, hilo_dispatch, NULL);
    
   // Crear hilo para recibir interrupciones
   pthread_create(&thread_interrupt, NULL, hilo_interrupt, NULL);
    
   // Crear hilo para ejecutar el ciclo de instrucción
   pthread_create(&thread_ciclo_instruccion, NULL, hilo_ciclo_instruccion, NULL);

   // Esperar a que los hilos terminen (en este caso, no se espera porque son hilos de ejecución continua)
   pthread_join(thread_dispatch, NULL);
   pthread_join(thread_interrupt, NULL);
   pthread_join(thread_ciclo_instruccion, NULL);

   log_info(logger_cpu, "Finalizando ejecución del CPU %d", id_cpu);

   close(socket_memoria);
   log_destroy(logger_sockets);
   destruir_configs();

   return 0;
}

void* hilo_dispatch(void* arg){
   while (1) {
      // ASIGNACION DE PROCESO
      t_paquete* paquete = recibir_paquete(socket_kernel_dispatch);
      if (paquete == NULL) {
         log_error(logger_cpu, "Error al recibir paquete de Kernel. Cerrando conexion");
         pthread_exit(NULL); // Termino el hilo si hay un error
      }
      if (paquete->codigo_operacion == PAQUETE_INSTRUCCION_CPU) {
         log_info(logger_cpu, "Recibido paquete de instrucción de Kernel");
         
         pthread_mutex_lock(&mutex_proceso); // lock(mutex_proceso)
         if (proceso != NULL) {
            liberar_proceso(proceso); // Liberar el proceso anterior si existe
         }
         proceso = deserializar_proceso_cpu(paquete->buffer);
         log_info(logger_cpu, "PID: %d, PC: %d", proceso->pid, proceso->pc);
         pthread_mutex_unlock(&mutex_proceso); // Liberar el mutex
      }
      liberar_paquete(paquete);
   }
}

void* hilo_interrupt(void* arg){
   while (1) {
      // Lógica para recibir interrupciones
      t_paquete* paquete_interrupt = recibir_paquete(socket_kernel_interrupt);
      if (paquete_interrupt == NULL) {
         log_error(logger_cpu, "Error al recibir interrupción de Kernel. Cerrando conexion");
         pthread_exit(NULL); // Terminar el hilo si hay un error
      }
      // Procesar la interrupción
      log_info(logger_cpu, "Recibida interrupción de Kernel");
      // Aquí puedes manejar la lógica de la interrupción según sea necesario
      liberar_paquete(paquete_interrupt);
   }
}

void* hilo_ciclo_instruccion(void* arg){
   // LIMPIAR TLB Y CACHE DE PAGINAS
   while (1) {
      // Adquirir el mutex antes de acceder a la variable proceso
      pthread_mutex_lock(&mutex_proceso);
      if (proceso != NULL) {
         // Ejecutar el ciclo de instrucción
         ciclo_instruccion(proceso, socket_memoria, socket_kernel_dispatch, socket_kernel_interrupt);
      }
      pthread_mutex_unlock(&mutex_proceso);
   }
}
