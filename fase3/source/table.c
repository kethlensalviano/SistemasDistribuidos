/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 3 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "table-private.h"
#include "list.h"
#include "list-private.h"
#include "entry.h"
#include "data.h"

/* Função para criar/inicializar uma nova tabela hash, com n
 * linhas (n = módulo da função hash)
 * Em caso de erro retorna NULL.
 */
struct table_t *table_create(int n){

    struct table_t * table;
    int i;
    table = (struct table_t *)malloc(sizeof(struct table_t));

    if(table != NULL) {
        table->list = (struct list_t **)malloc(sizeof(struct list_t)*n);
        if(table->list != NULL) {
            for(i = 0; i < n; i++) {
                table->list[i] = list_create();

            }
            table->size = n;
            return table;
        }
    }
    return NULL;
}

/* Função para libertar toda a memória ocupada por uma tabela.
 */
void table_destroy(struct table_t *table){

    int i;
    for(i=0; i < table->size; i++) {
        list_destroy(table->list[i]);
    }
    free(table->list);
    free(table);
}

/* Função para adicionar um par chave-valor à tabela.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na tabela,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int table_put(struct table_t *table, char *key, struct data_t *value){

    struct entry_t * entry;
    struct list_t* list;
    int index;
    index = hash(key, table->size);
    list = table->list[index];

    if(list == NULL) {  //caso lista ainda nao existe
        table->list[index] = list_create();
        list = table->list[index];
        if(table->list[index] == NULL) {
            return -1;
        }
    }
    entry = entry_create(key, value);
    struct data_t* datacpy = data_dup(value);
    entry->key = strdup(key);
    entry->value = datacpy;

    int result = list_add(list, entry);
    if(result == -1){
        return -1;
    }

    return 0;
}

/* Função para obter da tabela o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou table_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da tabela,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função.
 * Devolve NULL em caso de erro.
 */
struct data_t *table_get(struct table_t *table, char *key){

    struct entry_t *entry;
    int index = hash(key, table->size);

    if(table->list[index] != NULL) {
        entry = list_get(table->list[index], key);

        if(entry != NULL) {

            struct data_t *data = (struct data_t*)malloc(sizeof(struct data_t));
            if(data != NULL) {
                data->datasize = entry->value->datasize;

                data->data = (void*)malloc(entry->value->datasize);
                memcpy(data->data, entry->value->data, entry->value->datasize);
                return data;
            }
        }
    }
    return NULL;
}

/* Função para remover um elemento da tabela, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação table_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int table_del(struct table_t *table, char *key){

    if(table != NULL && key != NULL) {
        int index = hash(key, table->size);
        if(list_get(table->list[index], key) == NULL) { //nao existe
            return -1;
        }
        list_remove(table->list[index], key);
        return 0;
    }
    return -1;
}

/* Função que devolve o número de elementos contidos na tabela.
 */
int table_size(struct table_t *table){

    int i, size = 0;
    for(i = 0; i < table->size; i++) {
        if(table->list[i] != NULL) {
             size += list_size(table->list[i]);
        }
    }
    return size;
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
char **table_get_keys(struct table_t *table){

    char** keys = (char**)malloc((table_size(table)+1) * sizeof(char*));
    char** keys_list;
    int i, j, index = 0;
    if(keys != NULL) {
        for(i = 0; i < table->size; i++) {
            keys_list = list_get_keys(table->list[i]);

            for(j = 0; j < list_size(table->list[i]); j++) {
                keys[index] = strdup(keys_list[j]);
                index++;
            }
            list_free_keys(keys_list);
        }
        keys[index] = NULL;
        return keys;
    }
    return NULL;
}

/* Função que liberta toda a memória alocada por table_get_keys().
 */
void table_free_keys(char **keys){

    int i = 0;
    while(keys[i] != NULL) {
        free(keys[i]);
        i++;
    }
    free(keys);
}

/* Função que imprime o conteúdo da tabela.
 */
void table_print(struct table_t *table){

    if(table != NULL) {
        for(int i = 0; i < table->size; i++) {
            struct list_t* list = table->list[i];
            printf("index: %d\n", i);
            list_print(list);
        }
    }
}
