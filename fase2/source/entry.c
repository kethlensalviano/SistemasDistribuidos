/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA 
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 2 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835 
*   MANUEL LOPES - 49023
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entry.h"
#include "data.h"

/* Função que cria uma entry, reservando a memória necessária e
 * inicializando-a com a string e o bloco de dados passados.
 */
struct entry_t *entry_create(char *key, struct data_t *data){

    if(key == NULL || data == NULL) {
        return NULL;
    }

    struct entry_t *entry;
    entry = (struct entry_t *) malloc(sizeof(struct entry_t));

    if(entry != NULL) {
        entry->key = key;
        entry->value = data;
    }
    return entry;
}

/* Função que inicializa os elementos de uma entrada na tabela com o
 * valor NULL.
 */
void entry_initialize(struct entry_t *entry){

    if(entry != NULL) {
        entry->key = NULL;
        entry->value = NULL;
    }
}

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry){

    if(entry != NULL) {
        data_destroy(entry->value);
        free(entry->key);
        free(entry);
    }
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry){

    if(entry == NULL || entry->key == NULL) {
        return NULL;
    }
    char * key_dup;
    struct data_t * value_dup;

    key_dup = (char*) malloc(strlen(entry->key)+1);

    if(key_dup == NULL) {
        return NULL;
    }

    value_dup = data_dup(entry->value);
    strcpy(key_dup, entry->key);
    return entry_create(key_dup, value_dup);
}

/* Função que substitui o conteúdo de uma entrada entry_t.
*  Deve assegurar que destroi o conteúdo antigo da mesma.
*/
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value){

    if(entry != NULL && new_key != NULL) {
        free(entry->key);
        entry->key = new_key;
        data_replace(entry->value, new_value->datasize, new_value);
    }

}

/* Função que compara duas entradas e retorna a ordem das mesmas.
*  Ordem das entradas é definida pela ordem das suas chaves.
*  A função devolve 0 se forem iguais, -1 se entry1<entry2, e 1 caso contrário.
*/
int entry_compare(struct entry_t *entry1, struct entry_t *entry2){

    int result;
    result = strcmp(entry1->key, entry2->key);

    if(result > 0) {
        return 1;
    } else if(result < 0) {
        return -1;
    } else {
        return 0;
    }
}