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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "serialization.h"
#include "data.h"
#include "entry.h"

/* Serializa uma estrutura data num buffer que será alocado
 * dentro da função. Além disso, retorna o tamanho do buffer
 * alocado ou -1 em caso de erro.
 */
int data_to_buffer(struct data_t *data, char **data_buf){

    if(data == NULL || data_buf == NULL) {
        return -1;
    }

    int buf_size = sizeof(int) + data->datasize;
    *data_buf = (char*)malloc(buf_size);

    if(*data_buf == NULL) {
        return -1;
    }

    char *offset;
    offset = *data_buf;

    memcpy(offset, &data->datasize, sizeof(int));
    offset += sizeof(int);
    memcpy(offset, data->data, data->datasize);

    return buf_size;
}

/* De-serializa a mensagem contida em data_buf, com tamanho
 * data_buf_size, colocando-a e retornando-a numa struct
 * data_t, cujo espaco em memoria deve ser reservado.
 * Devolve NULL em caso de erro.
 */
struct data_t *buffer_to_data(char *data_buf, int data_buf_size){
    if(data_buf == NULL || data_buf_size < 0) {
        return NULL;
    }

    struct data_t *data;
    data = (struct data_t*)malloc(sizeof(struct data_t));

    char *offset;
    offset = data_buf;

    if(data == NULL) {
        return NULL;
    }

    data->datasize = data_buf_size - sizeof(int);
    data->data = (void*)malloc(data->datasize);
    //memcpy(data->datasize, &data_buf, sizeof(int));
    offset += sizeof(int);
    if(data->data == NULL) {
        return NULL;
    }

    memcpy(data->data, offset, data->datasize);
    return data;

}

/* Serializa uma estrutura entry num buffer que sera alocado
 * dentro da função. Além disso, retorna o tamanho deste
 * buffer alocado ou -1 em caso de erro.
 */
int entry_to_buffer(struct entry_t *data, char **entry_buf){

    if(data == NULL || entry_buf == NULL) {
        return -1;
    }

    int len_key = strlen(data->key)+1;
    int buf_size = sizeof(int) + strlen(data->key)+1 + sizeof(int) + data->value->datasize;

    *entry_buf = (char*) malloc(buf_size);

    char *offset;
    offset = *entry_buf;

    memcpy(offset, &len_key, sizeof(int)); //key size to buffer
    offset += sizeof(int);
    memcpy(offset, data->key, len_key); //key to buffer
    offset += len_key;
    memcpy(offset, &data->value->datasize, sizeof(int)); //data size to buffer
    offset += sizeof(int);
    memcpy(offset, data->value->data, data->value->datasize); //data to buffer
    return buf_size;

}

/* De-serializa a mensagem contida em entry_buf, com tamanho
 * entry_buf_size, colocando-a e retornando-a numa struct
 * entry_t, cujo espaco em memoria deve ser reservado.
 * Devolve NULL em caso de erro.
 */
struct entry_t* buffer_to_entry(char* entry_buf, int entry_buf_size) {

    if (entry_buf == NULL || entry_buf_size < 0) return NULL;

    int offset = 0;

    int key_size = *(int*)(entry_buf);
    char* key = malloc(key_size + 1);
    memcpy(key, entry_buf + sizeof(int), key_size);
    offset += sizeof(int) + key_size;

    int datasize = *(int*)(entry_buf + offset);
    offset += sizeof(int);

    struct entry_t* entry = (struct entry_t*)malloc(sizeof(struct entry_t));
    entry->key = key;

    struct data_t* data = (struct data_t*)malloc(sizeof(struct data_t));
    data->datasize = datasize;
    void* d = malloc(datasize);
    memcpy(d, entry_buf + offset, datasize);

    data->data = d;
    entry->value = data;

    return entry;

}
