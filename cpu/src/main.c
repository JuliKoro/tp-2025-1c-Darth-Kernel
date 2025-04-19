#include <utils/sockets.h>
#include <utils/hello.h>
#include <utils/handshake.h>
#include <utils/configs.h>
#include "cpu-conexiones.h"

int main(int argc, char* argv[]) {
    saludar("cpu");

    cpu_conectar_a_kernel();

    //cpu_conectar_a_memoria();

    return 0;
}