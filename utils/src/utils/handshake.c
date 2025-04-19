#include "handshake.h"

const char* nombre_modulo(id_modulo_t modulo){
    switch (modulo) {
        case HANDSHAKE_CPU: return "CPU";
        case HANDSHAKE_KERNEL: return "KERNEL";
        case HANDSHAKE_MEMORIA: return "MEMORIA";
        case HANDSHAKE_IO: return "IO";
        default: return "DESCONOCIDO";

    }
}
//" Se recibio conexion de modulo: %s (ID %s)",nombre_modulo(modulo), modulo);

int enviar_handshake (int socket, id_modulo_t modulo, t_log* logger) {
    //Casteo enum a uint8_t
    u_int8_t id = (u_int8_t)modulo;
    //Envio al socket el valor de id, chequeo errores
    if(send(socket, &id, sizeof(u_int8_t), 0) == -1){
        log_error(logger,"Error al enviar handshake %s (ID %d) : %s", nombre_modulo(modulo), modulo , strerror(errno));
        return -1;
    } 

    log_info(logger, "Handshake de %s (ID %d) enviado!", nombre_modulo(modulo), modulo);

    return 0;
}

int recibir_handshake (int socket, id_modulo_t* modulo_recibido, t_log* logger){
    u_int8_t id;

    //Recibo valor de id y chequeo errores
    if(recv(socket, &id, sizeof(u_int8_t), MSG_WAITALL) == -1){
        log_error(logger,"Error al recibir handshake: %s", strerror(errno));
        return -1;
    }

    //Casteo el u_int8_t a id_modulo_t y la almaceno en la variable que se recibio para eso
    *modulo_recibido = (id_modulo_t)id;
    
    log_info(logger, "Handshake de %s (ID %d) recibido!", nombre_modulo((id_modulo_t)id), id);
    
    return 0;
}