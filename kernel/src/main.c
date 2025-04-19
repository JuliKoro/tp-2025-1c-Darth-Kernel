#include <utils/sockets.h>
#include <utils/handshake.h>
#include <utils/hello.h>
#include <utils/configs.h>
#include "kernel-conexiones.h"

int main(int argc, char* argv[]) {
    saludar("kernel");

    //kernel_conectar_a_cpu();
    kernel_conectar_a_io();
    //kernel_conectar_a_memoria();


    return 0;
}
