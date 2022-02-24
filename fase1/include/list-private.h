/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA 
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 1 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835 
*   MANUEL LOPES - 49023
*/

#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "list.h"

struct node_t {
    struct entry_t* entry;
    struct node_t* next;

};

struct list_t {
    struct node_t* head;

};

void node_destroy(struct node_t* node);

int key_exists(struct node_t* node, struct entry_t* entry);

#endif
