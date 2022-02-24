/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 3 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/

#include "message.h"
#include "entry.h"
#include "data.h"
#include "table.h"
#include "stats-private.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//não funciona
int read_all(int sock,  void*in_buf, int len) {
    int bufsize = len;
    while(len>0) {
        int res = read(sock, in_buf, len);
        if(res<=0) {
            if(errno==EINTR) continue;
            //perror("read failed:");
            return res;
        }
        in_buf += res;
        len -= res;
    }
    return bufsize;
}

int write_all(int sock, void *out_buf, int len) {
    int bufsize = len;
    while(len>0) {
        int res = write(sock, out_buf, len);
        if(res<0) {
            if(errno==EINTR) continue;
            perror("write failed:");
            return res;
        }
        out_buf += res;
        len -= res;
    }
    return bufsize;
}

int replace_message(int opcode, int ctype, MessageT* msg) {

    if(msg == NULL) return -1;
    msg->opcode = opcode;
    msg->c_type = ctype;

    return 0;
}

int data_to_message(struct data_t *data, MessageT* msg) {

    //if(data == NULL) return -1;

    msg->data_size = data->datasize;

    if(data->data != NULL){
        void *copy = malloc(data->datasize);
        if(copy != NULL) {
            memcpy(copy, data->data, data->datasize);
        }

        msg->data = copy;
    }


    return 0;
}

struct entry_t* message_to_entry(MessageT* msg) {

    struct entry_t *entry;
    struct data_t *value;
    int datasize;

    datasize = msg->data_size;
    void *data = malloc(datasize);

    if(data != NULL) {
        memcpy(data, msg->data, msg->data_size);
    }

    char *key = strdup(msg->key);
    value = data_create2(datasize, data);
    entry = entry_create(key, value);

    return entry;
}

MessageT* entry_to_message(struct entry_t* entry) {

    struct MessageT* msg = init_message(OP_PUT, CT_ENTRY);

    void* copy = (void *)malloc(entry->value->datasize);
    if(copy == NULL) return NULL;

    memcpy(copy, entry->value->data, entry->value->datasize);

    msg->key = strdup(entry->key);
    msg->data = copy;
    msg->data_size = entry->value->datasize;

    return msg;
}

MessageT* key_to_message(char* key, int opcode) {
    MessageT* msg = init_message(opcode, CT_KEY);
    msg->key = strdup(key);

    return msg;
}

int keys_to_message(MessageT* msg, char ** keys) {
    int size = 0;

    while(keys[size] != NULL) {
        size++;
    }

    msg->n_keys = size;
    msg->keys = keys;

    return 0;

}

int table_to_message(MessageT* msg, struct table_t *table){
  int res = replace_message(OP_PRINT+1, CT_TABLE, msg);
  int size = table_size(table);
  struct data_t *data;

  char **values = (char**) malloc(sizeof(char *) * (size+1));
  char **keys = table_get_keys(table);

  for(int i = 0; i < size; i++){
    data = table_get(table,keys[i]);
    values[i] = strdup(data->data);
    data_destroy(data);
  }

  msg->n_keys = size;
  msg->n_values = size;
  msg->keys = keys;
  msg->values = values;

  return res;
}

int stats_to_message(MessageT* msg,struct statistics* stats_struct){
  int res = replace_message(OP_STATS+1, CT_RESULT, msg);
  int size = 6;
  int *stats = malloc(sizeof(int)*size);
  msg->n_stats = size;

  stats[0] = stats_struct->size_count;
  stats[1] = stats_struct->del_count;
  stats[2] = stats_struct->get_count;
  stats[3] = stats_struct->put_count;
  stats[4] = stats_struct->getkeys_count;
  stats[5] = stats_struct->table_print_count;
  msg->stats = stats;
  msg->avg_response = stats_struct->avg_response;

  return res;
}

MessageT* init_message(int opcode, int ctype) {
    struct MessageT* msg = (MessageT*) malloc(sizeof(MessageT));
    if(msg == NULL) return NULL;

    message_t__init(msg);
    msg->opcode = opcode;
    msg->c_type = ctype;

    return msg;
}
