#include "listas.h"

void* list_find_con_param(t_list* lista, bool (*comparar)(void*, void*), void* param) {
    for(int i = 0; i < list_size(lista); i++) {
        void* elemento = list_get(lista, i);
        if(comparar(elemento, param)) {
            return elemento;
        }
    }
    return NULL;
}