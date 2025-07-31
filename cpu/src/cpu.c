#include "cpu.h"

int socket_kernel_dispatch;
int socket_kernel_interrupt;
int socket_memoria;

// Mutex para proteger el acceso a la variable proceso
pthread_mutex_t mutex_proceso = PTHREAD_MUTEX_INITIALIZER; // Inicialización estática

sem_t semaforo_proceso; // Semáforo para controlar la recepción de nuevos procesos
sem_t semaforo_interrupcion; // Semáforo para controlar la recepción de interrupciones

t_proceso_cpu* proceso = NULL;
t_interrupcion* interrupcion = NULL;

// REGISTROS
uint32_t PC; // Declaracion de la variable global para el PC (Porgram Counter)
bool IF = 0; // Declaracion de la pariable global para el IF (Interrupt Flag)

int id_cpu; //La hice global para que se pueda usar en los hilos de dispatch e interrupt.

int main(int argc, char* argv[]) {
   
   // INICIO CPU
   // Verifco que se haya pasado el ID como argumento
   if (argc != 2) { 
      printf("Uso CPU: %s [identificador]\n", argv[0]);
      return EXIT_FAILURE;
   }

   // Guardo el id del argumento
   id_cpu = atoi(argv[1]);
   if (id_cpu <= 0) {
      fprintf(stderr, "El ID del CPU debe ser un número positivo.\n");
      return EXIT_FAILURE;
   }

   inicializacion_cpu();

   if(conexiones_cpu() == -1) return EXIT_FAILURE;

   // HILOS
   pthread_t thread_dispatch, thread_interrupt, thread_ciclo_instruccion;

   // Crear hilo para recibir instrucciones del kernel
   pthread_create(&thread_dispatch, NULL, hilo_dispatch, NULL);
    
   // Crear hilo para recibir interrupciones
   pthread_create(&thread_interrupt, NULL, hilo_interrupt, NULL);
    
   // Crear hilo para ejecutar el ciclo de instrucción
   pthread_create(&thread_ciclo_instruccion, NULL, hilo_ciclo_instruccion, NULL);

   // Inicializaion de Semaforos
   sem_init(&semaforo_proceso, 0, 0); // Inicializa el semáforo en 0 (entre hilos)
   sem_init(&semaforo_interrupcion, 0, 0);

   // Esperar a que los hilos terminen (en este caso, no se espera porque son hilos de ejecución continua)
   pthread_join(thread_dispatch, NULL);
   pthread_join(thread_interrupt, NULL);
   pthread_join(thread_ciclo_instruccion, NULL);

   finalizacion_cpu();

   return 0;
}

void* hilo_dispatch(void* arg){

   //Hago el handshake con el kernel
   if(enviar_handshake_cpu(socket_kernel_dispatch, id_cpu) == -1){
      log_error(logger_cpu, "Error al enviar handshake a Kernel. Cerrando conexion");
      pthread_exit(NULL);
   }

   log_debug(logger_cpu, "Handshake enviado correctamente. Esperando confirmacion de kernel...");

   //Recibo la confirmacion de kernel que el socket de dispatch fue creado y asignado a la cpu
   char* mensaje = recibir_mensaje(socket_kernel_dispatch);
   if(mensaje == NULL){
      log_error(logger_cpu, "Error al recibir mensaje de Kernel. Cerrando conexion");
      pthread_exit(NULL);
   }

   log_debug(logger_cpu, "Mensaje recibido del kernel: %s", mensaje);
   free(mensaje);

   while (1) {
      // ASIGNACION DE PROCESO
      t_paquete* paquete = recibir_paquete(socket_kernel_dispatch);
      if (paquete == NULL) {
         log_error(logger_cpu, "Error al recibir paquete de Kernel. Cerrando conexion");
         pthread_exit(NULL); // Termino el hilo si hay un error
      }
      if (paquete->codigo_operacion == PAQUETE_PROCESO_CPU) {
         log_debug(logger_cpu, "Recibido paquete de instrucción de Kernel");
         
         pthread_mutex_lock(&mutex_proceso); // lock(mutex_proceso)
         // No hace falta liberar proceso (si es que habia uno anterior) xq está declarada de forma global (memoria asignada estáticamente)
         proceso = deserializar_proceso_cpu(paquete->buffer);
         log_debug(logger_cpu, "PID: %d, PC: %d", proceso->pid, proceso->pc);
         pthread_mutex_unlock(&mutex_proceso); // Liberar el mutex
         // Esperar a que el proceso ejecute su ciclo_instruccion (hasta que termine (syscall) o se interrumpa)
         sem_wait(&semaforo_proceso);
      }
      
      liberar_paquete(paquete);
   }
}

void* hilo_interrupt(void* arg){

   //Hago el handshake con el kernel
   if(enviar_handshake_cpu(socket_kernel_interrupt, id_cpu) == -1){
      log_error(logger_cpu, "Error al enviar handshake a Kernel. Cerrando conexion");
      pthread_exit(NULL);
   }

   log_debug(logger_cpu, "Handshake enviado correctamente. Esperando confirmacion de kernel...");

   //Recibo la confirmacion de kernel que el socket de interrupt fue creado y asignado a la cpu
   char* mensaje = recibir_mensaje(socket_kernel_interrupt);
   if(mensaje == NULL){
      log_error(logger_cpu, "Error al recibir mensaje de Kernel. Cerrando conexion");
      pthread_exit(NULL);
   }

   log_debug(logger_cpu, "Mensaje recibido del kernel: %s", mensaje);
   free(mensaje);

   while (1) {
      // Lógica para recibir interrupciones
      t_paquete* paquete_interrupt = recibir_paquete(socket_kernel_interrupt);
      if (paquete_interrupt == NULL) {
         log_error(logger_cpu, "Error al recibir interrupción de Kernel. Cerrando conexion");
         pthread_exit(NULL); // Terminar el hilo si hay un error
      }
      if (paquete_interrupt->codigo_operacion == PAQUETE_INTERRUPCION) {
         log_info(logger_cpu, "## Llega interrupción al puerto Interrupt");
         interrupcion = deserializar_interrupcion(paquete_interrupt->buffer);
         // Procesar la interrupción 
         IF = 1; // Flag: Indica que se recibio una interrupcion
         
         sem_wait(&semaforo_interrupcion);
         
      }
      
      liberar_paquete(paquete_interrupt);
   }
}

void* hilo_ciclo_instruccion(void* arg){
   while (1) {
      // Adquirir el mutex antes de acceder a la variable proceso
      pthread_mutex_lock(&mutex_proceso);
      if (proceso != NULL) {
         // Ejecutar el ciclo de instrucción
         ciclo_instruccion(proceso, interrupcion, socket_memoria, socket_kernel_dispatch, socket_kernel_interrupt);
      }
      pthread_mutex_unlock(&mutex_proceso);

      // DESALOJO DE PROCESO
      desalojar_proceso(proceso->pid);

      // Libera el semáforo para permitir que el hilo de despacho reciba un nuevo proceso
      sem_post(&semaforo_proceso);
      // Libera el semáforo para permitir que el hilo de interrupt reciba una nueva interrupcion
      sem_post(&semaforo_interrupcion);
   }
}

void inicializacion_cpu(){
   // CARGA CONFIGS
   inicializar_configs(); // Se carga la variable global 'cpu_configs'

   // INICIO LOGGERS
   logger_cpu = iniciar_logger_cpu(id_cpu); // crea logger cpu_<id>.log
   log_debug(logger_cpu, "Iniciando CPU con ID: %d", id_cpu);

   cache = crear_cache(); // INICIO CACHE
   tlb = crear_tlb(); // INICIO TLB
}

int conexiones_cpu(){
   // SOCKETS
   // Conexiones al Kernel (dispatch & interrupt)
   socket_kernel_dispatch = cpu_conectar_a_kernel(cpu_configs.puertokerneldispatch, id_cpu);
   if (socket_kernel_dispatch < 0) {
      log_error(logger_cpu, "[CPU] Error al conectar al Kernel para dispatch");
      return -1;
   }
   socket_kernel_interrupt = cpu_conectar_a_kernel(cpu_configs.puertokernelinterrupt, id_cpu);
   if (socket_kernel_interrupt < 0) {
      log_error(logger_cpu, "[CPU] Error al conectar al Kernel para interrupt");
      return -1;
   }

   // Conexion con Memoria
   socket_memoria = cpu_conectar_a_memoria(id_cpu); // cambiar handshake con memoria por info de tablas de paginas
   if (socket_memoria < 0) {
      log_error(logger_cpu, "[CPU] Error al conectar con Memoria");
      return -1;
   }
   t_tabla_pag* info_tabla_pag = hanshake_cpu_memoria(socket_memoria, id_cpu);
   cargar_configs_tabla_paginas(info_tabla_pag);
}

void desalojar_proceso(uint32_t pid){
   // LIMPIAR CACHE DE PAGINAS
   if (acceder_cache()) { // Si esta habiliatada la Cache
      actualizar_cache_a_memoria(pid, socket_memoria);
      limpiar_cache();
   }

   // LIMPIAR TLB
   limpiar_tlb(pid);
   log_debug(logger_cpu, "Proceso desalojado: PID: %d", pid);

   proceso = NULL;
}

void finalizacion_cpu(){
   log_debug(logger_cpu, "Finalizando ejecución del CPU %d", id_cpu);

   close(socket_memoria);
   destruir_cache();
   destruir_tlb();
   //log_destroy(logger_sockets);
   destruir_configs();
}