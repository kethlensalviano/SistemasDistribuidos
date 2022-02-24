/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 3 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/

#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "sdmessage.pb-c.h"
#include "table.h"
#include "stats-private.h"

/* Define os possíveis opcodes da mensagem */
#define OP_SIZE 10
#define OP_DEL 20
#define OP_GET 30
#define OP_PUT 40
#define OP_GETKEYS 50
#define OP_PRINT 60
#define OP_STATS 70
/* opcode para representar retorno de erro da execução da operação*/
#define OP_ERROR 99
/* Define códigos para os possíveis conteúdos da mensagem (c_type) */
#define CT_KEY 10
#define CT_VALUE 20
#define CT_ENTRY 30
#define CT_KEYS 40
#define CT_RESULT 50
#define CT_TABLE 60
/* c_type representativo de inexistência de content
* (e.g., getkeys, size e table_print */
#define CT_NONE 70

//struct message_t {
//   struct MessageT *m;
//};

int read_all(int sock, void *buf, int len);

int write_all(int sock, void *buf, int len);

int replace_message(int opcode, int ctype, MessageT* msg);

int data_to_message(struct data_t *data, MessageT* msg);

int keys_to_message(MessageT* msg, char ** keys);

struct entry_t* message_to_entry(MessageT* msg);

MessageT* entry_to_message(struct entry_t* entry);

MessageT* key_to_message(char* key, int opcode);

MessageT* init_message(int opcode, int ctype);

int table_to_message(MessageT* msg, struct table_t *table);

int stats_to_message(MessageT* msg,struct statistics* stats_struct);

#endif
