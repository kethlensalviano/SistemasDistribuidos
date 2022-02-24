/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA 
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 2 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835 
*   MANUEL LOPES - 49023
*/

#include "message.h"
#include "entry.h"
#include "data.h"

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
    msg->keys = malloc(sizeof(char *) * size);

    if(msg->keys == NULL) return -1;

    for(int i = 0; i < size; i++) {
        msg->keys[i] = strdup(keys[i]);
    }

    return 0;
    
}

MessageT* init_message(int opcode, int ctype) {
    struct MessageT* msg = (MessageT*) malloc(sizeof(MessageT));
    if(msg == NULL) return NULL;

    message_t__init(msg);
    msg->opcode = opcode;
    msg->c_type = ctype;

    return msg;
}