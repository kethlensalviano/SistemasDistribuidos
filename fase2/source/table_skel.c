/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA 
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 2 DO PROJETO
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

#include <stdio.h>

struct table_t *table;

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists) {

    if (n_lists > 0) {
        table = table_create(n_lists);
        return 0;
    }

    return -1;

}

/* Liberta toda a memória e recursos alocados pela função table_skel_init.
 */
void table_skel_destroy() {
    table_destroy(table);

}

/* Executa uma operação na tabela (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, tabela nao incializada)
*/
int invoke(MessageT *msg) {

   // printf("servidor recebeu pedido: %d \n", msg->opcode);

    if(msg == NULL) {
        return -1;
    }

    if(table == NULL) {
        return -1;
    }

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
            table_free_keys(keys);
            break;

        case OP_PRINT:
            res = replace_message(OP_PRINT+1, CT_TABLE, msg);
            break;
            
        default:
            break;
    }

    return res;

}
