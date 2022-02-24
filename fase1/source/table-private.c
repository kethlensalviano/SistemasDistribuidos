/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA 
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 1 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835 
*   MANUEL LOPES - 49023
*/

#include <string.h>
#include <stdio.h>

#include "table-private.h"

int hash(char *key, int n) {
    
    int len, soma, i = 0;
    len = strlen(key);
    soma = 0;
    for(i = 0; i < len; i++) {
        soma = soma + key[i];
    }
    return soma % n;
}