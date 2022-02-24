/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA 
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 1 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835 
*   MANUEL LOPES - 49023
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list-private.h"

void node_destroy(struct node_t* node) {

    if(node != NULL) {
        entry_destroy(node->entry);
        free(node);
    }
}

int key_exists(struct node_t* node, struct entry_t* entry) {

    struct entry_t *p;

    while(node != NULL) {
        int result = strcmp(node->entry->key, entry->key);
        if(result == 0) { //sao iguais e substitui a entry
            p = node->entry;
            node->entry = entry;
            entry_destroy(p);
            return 0;
        }
        node = node->next;
    }
    return -1;
}
