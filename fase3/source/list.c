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

#include "list.h"
#include "list-private.h"
#include "data.h"

/* Função que cria uma nova lista (estrutura list_t a ser definida pelo
 * grupo no ficheiro list-private.h).
 * Em caso de erro, retorna NULL.
 */
struct list_t *list_create(){

    struct list_t* new_list;
    new_list = (struct list_t *) malloc(sizeof(struct list_t));

    if(new_list != NULL){
        new_list->head = NULL;
        return new_list;
    }
    return NULL;
}

/* Função que elimina uma lista, libertando *toda* a memoria utilizada
 * pela lista.
 */
void list_destroy(struct list_t *list){

    struct node_t * temp, * p;
    temp = list->head;

    while(temp != NULL) {
        p = temp->next;
        node_destroy(temp);
        temp = p;
    }
    free(list);
}

/* Função que adiciona no final da lista (tail) a entry passada como
* argumento caso não exista na lista uma entry com key igual àquela
* que queremos inserir.
* Caso exista, os dados da entry (value) já existente na lista serão
* substituídos pelos os da nova entry.
* Retorna 0 (OK) ou -1 (erro).
*/

int list_add(struct list_t *list, struct entry_t *entry){

    struct node_t * temp;
    temp = list->head;

    if(list != NULL) {

        if(key_exists(temp, entry) == 0) {
            return 0;
        } else {

            //creates new node
            struct node_t * new_node = (struct node_t*) malloc(sizeof(struct node_t));
            temp = list->head;

            if(new_node != NULL) {
                new_node->entry = entry;
                new_node->next = NULL;

                if(list->head == NULL) { //caso head
                    list->head = new_node;
                    return 0;
                } else {
                    while(temp->next != NULL) {
                        temp = temp->next;
                    }
                    temp->next = new_node;
                    return 0;
                }
            }
        }
    }
    return -1;
}

/* Função que elimina da lista a entry com a chave key.
 * Retorna 0 (OK) ou -1 (erro).
 */
int list_remove(struct list_t *list, char *key){

    struct node_t* temp, *previous, *p;
    temp = list->head;

    if(list != NULL && key != NULL) {

        if(strcmp(temp->entry->key, key) == 0 ) {  //caso head
            p = temp->next;
            node_destroy(temp);
            list->head = p;
            return 0;
        } else {
            while(temp != NULL) {
                int result = strcmp(temp->entry->key, key);
                if(result == 0 ) {
                    previous->next = temp->next;
                    node_destroy(temp);
                    return 0;
                }
                previous = temp;
                temp = temp->next;
            }
        }
    }
    return -1;
}

/* Função que obtém da lista a entry com a chave key.
 * Retorna a referência da entry na lista ou NULL em caso de erro.
 * Obs: as funções list_remove e list_destroy vão libertar a memória
 * ocupada pela entry ou lista, significando que é retornado NULL
 * quando é pretendido o acesso a uma entry inexistente.
*/
struct entry_t *list_get(struct list_t *list, char *key){

    struct node_t* temp;
    temp = list->head;

    if(list != NULL && key != NULL) {
        while(temp != NULL) {
            int result = strcmp(temp->entry->key, key);
            if(result == 0){
                return temp->entry;
            }
            temp = temp->next;
        }
    }
    return NULL;
}

/* Função que retorna o tamanho (número de elementos (entries)) da lista,
 * ou -1 (erro).
 */
int list_size(struct list_t *list){

    if(list != NULL) {
        int size = 0;
        struct node_t * temp;
        temp = list->head;

        while(temp != NULL) {
            size++;
            temp = temp->next;
        }
        return size;
    }
    return -1;
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
char **list_get_keys(struct list_t *list){

    char** keys = (char**)malloc((list_size(list)+1) * sizeof(char*));

    if(keys != NULL) {
        int i = 0;
        struct node_t* temp;
        temp = list->head;

        while(temp != NULL) {
            keys[i] = (char*) malloc(strlen(temp->entry->key)+1);
            if(keys[i] != NULL) {
                strcpy(keys[i], temp->entry->key);
                i++;
                temp = temp->next;
            }
        }
        keys[i] = NULL;
        return keys;
    }
    return NULL;
}

/* Função que liberta a memória ocupada pelo array das keys da tabela,
 * obtido pela função list_get_keys.
 */
void list_free_keys(char **keys){

    int i = 0;
    while(keys[i] != NULL) {
        free(keys[i]);
        i++;
    }
    free(keys);

}

/* Função que imprime o conteúdo da lista para o terminal.
 */
void list_print(struct list_t *list){

    struct node_t *temp;
    temp = list->head;
    printf("start\n");
    while(temp != NULL) {
        printf("%s.  %s\n", temp->entry->key, (char *) temp->entry->value->data); //how to print data void * temp->entry->value->data
        temp = temp->next;
    }
    printf("end\n");
    printf("\n");
}
