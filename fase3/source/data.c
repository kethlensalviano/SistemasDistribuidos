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

#include "data.h"

/* Função que cria um novo elemento de dados data_t e reserva a memória
 * necessária, especificada pelo parâmetro size
 */
struct data_t* data_create(int size) {

    if (size < 0) return NULL;

    //if (size > 0) { /*caso normal- coloca o tamanho da memoria alocada no parametro datasize*/
    struct data_t * new; /* cria um novo  elemento de data_t data_t */
    new = (struct data_t *) malloc(sizeof (struct data_t));  /* garantir que reserva espaco para um novo elemento*/

    if(new != NULL) {
        new->datasize = size;
        if(size != 0) {
            new->data = (void*)malloc(size);
            return new;
        } else {
            new->data = NULL;
            return new;
        }

    }
    return NULL;
}

/* Função idêntica à anterior, mas que inicializa os dados de acordo com
 * o parâmetro data.
 */
struct data_t* data_create2(int size, void* data) {

    if(size > 0 && data != NULL) {
        struct data_t* new;
        new = data_create(size);
        free(new->data);
        new->data = data;
        return new;
    } else {
        struct data_t* new = data_create(0);
        return new;
    }
    return NULL;
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 */
void data_destroy(struct data_t* data) {

    if (data != NULL) {
        if(data->data != NULL) {
            free(data->data);
        }
    }
    free(data);
}

/* Função que duplica uma estrutura data_t, reservando a memória
 * necessária para a nova estrutura.
 */
struct data_t* data_dup(struct data_t* data) {

    if (data == NULL || data->datasize <= 0 || data->data == NULL) {
        return NULL;
    }
    void* c = malloc(data->datasize);
    memcpy(c, data->data, data->datasize);
    return data_create2(data->datasize, c);
}

/* Função que substitui o conteúdo de um elemento de dados data_t.
*  Deve assegurar que destroi o conteúdo antigo do mesmo.
*/
void data_replace(struct data_t *data, int new_size, void *new_data){

    if(data != NULL && new_data != NULL && new_size > 0) {
        free(data->data);
        data->datasize = new_size;
        data->data = (void*)malloc(new_size);
        data->data = new_data;
    }
}
