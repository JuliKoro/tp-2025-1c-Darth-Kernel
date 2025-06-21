#include "cpu-log.h"

t_log* iniciar_logger_cpu(int id_cpu){
    char log_filename[64]; // Crear nombre de log dinámico (log por cada CPU)
    snprintf(log_filename, sizeof(log_filename), "cpu_%d.log", id_cpu); // formato nombre del archvio 
    t_log* logger_cpu = iniciar_logger(log_filename, "[CPU]"); // crea logger cpu_<id>.log
    if (logger_cpu == NULL) {
        fprintf(stderr, "Error al crear el logger.\n");
        return EXIT_FAILURE;
    }
    return logger_cpu;
 }

 void destruir_logger_cpu() {
    //Chequeo que el logger exista y lo destruyo
    if (logger_cpu != NULL) {
        log_destroy(logger_cpu);
    }
}

