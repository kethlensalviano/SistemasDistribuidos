/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 3 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/

#include "sdmessage.pb-c.h"
#include "table.h"
#include "message.h"
#include "data.h"
#include "entry.h"
#include "table_skel_private.h"
#include "stats-private.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


struct table_t *table;
struct statistics *stats_struct;
extern int write_thread;
extern int write_stats;

pthread_mutex_t mutex_read = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_write = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_read_stats = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_write_stats = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_read = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_write = PTHREAD_COND_INITIALIZER;

pthread_cond_t cond_read_stats = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_write_stats = PTHREAD_COND_INITIALIZER;
/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists) {

    if (n_lists > 0) {
        table = table_create(n_lists);
        stats_struct = stats_create();
        return 0;
    }

    return -1;

}

/* Liberta toda a memória e recursos alocados pela função table_skel_init.
 */
void table_skel_destroy() {
    table_destroy(table);
    stats_destroy(stats_struct);

}

int actions(MessageT *msg) {

    int res = 0;
    int size;
    struct data_t *data;
    struct entry_t *entry;
    char** keys;

    switch(msg->opcode) {
        case OP_SIZE:
            size = table_size(table);
            if((res = replace_message(OP_SIZE+1, CT_RESULT, msg)) == 0) {
                msg->result = size;
            }
            break;

        case OP_DEL:
            if(table_del(table, msg->key) == 0) {
                res = replace_message(OP_DEL+1, CT_NONE, msg);
            } else {
                res = replace_message(OP_ERROR, CT_NONE, msg);
            }
            break;

        case OP_GET:
            data = table_get(table, msg->key);
            res = replace_message(OP_GET+1, CT_VALUE, msg);

            if(data != NULL) {
                res = data_to_message(data, msg);
            } else {
                data = data_create(0);
                res = data_to_message(data, msg);
            }
            data_destroy(data);
            break;

        case OP_PUT:

            entry = message_to_entry(msg);
            if(table_put(table, entry->key, entry->value) == 0) {
                res = replace_message(OP_PUT+1, CT_NONE, msg);
            } else {
                res = replace_message(OP_ERROR, CT_NONE, msg);
            }
            entry_destroy(entry);
            break;

        case OP_GETKEYS:
            keys = table_get_keys(table);
            if(keys != NULL){
                res = replace_message(OP_GETKEYS+1, CT_KEYS, msg);
            } else {
                res = replace_message(OP_ERROR, CT_NONE, msg);
            }

            res = keys_to_message(msg, keys);

            break;

        case OP_PRINT:
            res = table_to_message(msg,table);
            break;

        case OP_STATS:

            //le estrutura statistics e constroi mensagem
            read_mutex_stats(msg);

            break;

        default:
            break;
    }

    return res;

}

int write_mutex(MessageT *msg) {
    int res = 0;

    pthread_mutex_lock(&mutex_write);

    while(write_thread == 1) {
        pthread_cond_wait(&cond_write, &mutex_write);
    }

    pthread_mutex_unlock(&mutex_write);

    pthread_mutex_lock(&mutex_read);

    write_thread = 1;
    //do some writing
    //sleep(5);
    res = actions(msg);
    write_thread = 0;

    pthread_cond_broadcast(&cond_read);
    pthread_cond_broadcast(&cond_write);

    pthread_mutex_unlock(&mutex_read);
    return res;
}

int read_mutex(MessageT *msg) {
    int res = 0;

    pthread_mutex_lock(&mutex_read);

    while(write_thread == 1) {
        pthread_cond_wait(&cond_read, &mutex_read);
    }

    pthread_mutex_unlock(&mutex_read);

    //do some reading
    //sleep(2);
    res = actions(msg);

    return res;
}

int screening(MessageT *msg) {
    int res = 0;
    if(msg->opcode == OP_PUT || msg->opcode == OP_DEL) {
        res = write_mutex(msg);
    } else {
        res = read_mutex(msg);
    }

    return res;

}

int write_mutex_stats(int opcode,double resp_time) {
    int res = 0;

    pthread_mutex_lock(&mutex_write_stats);

    while(write_thread == 1) {
        pthread_cond_wait(&cond_write_stats, &mutex_write_stats);
    }

    pthread_mutex_unlock(&mutex_write_stats);

    pthread_mutex_lock(&mutex_read_stats);

    write_stats = 1;

    if (stats_increment(opcode,stats_struct) == 0) {
        //sleep(5);
        //printf("resp_time: %f\n",resp_time);
        stats_recalc_avg(stats_struct,resp_time);
    }

    write_stats = 0;

    pthread_cond_broadcast(&cond_read_stats);
    pthread_cond_broadcast(&cond_write_stats);

    pthread_mutex_unlock(&mutex_read_stats);
    return res;
}

int read_mutex_stats(MessageT *msg) {
    int res = 0;

    pthread_mutex_lock(&mutex_read_stats);

    while(write_stats == 1) {
        pthread_cond_wait(&cond_read_stats, &mutex_read_stats);
    }

    pthread_mutex_unlock(&mutex_read_stats);

    //do some reading and construct message
    if((res = stats_to_message(msg,stats_struct)) != 0) {
        //sleep(2);
        res = replace_message(OP_ERROR, CT_NONE, msg);
    }

    return res;
}

/* Executa uma operação na tabela (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, tabela nao incializada)
*/
int invoke(MessageT *msg) {

    int res;

    if(msg == NULL) {
        printf("Msg veio Null\n");
        return -1;
    }

    if(table == NULL) {
        printf("table veio Null\n");
        return -1;
    }

    // triagem do tipo de pedido para saber se chama o write_mutex ou read_mutex
    // write -> put, del
    // read -> size, get, getkeys, print

    if ((res = screening(msg)) != 0) {
      printf("Res veio !=0 do screening\n");
      return res;
    }

    return res;

}
