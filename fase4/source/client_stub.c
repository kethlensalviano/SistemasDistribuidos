/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 2 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/
#include "data.h"
#include "entry.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "message.h"
#include "stats-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "inet.h"

struct rtable_t *get_rtable(){
    struct rtable_t *rtable = (struct rtable_t *) malloc(sizeof(struct rtable_t));
    if(rtable != NULL){
      return rtable;
    }

    return NULL;
}

/* Remote table, que deve conter as informações necessárias para estabelecer a comunicação com o servidor. A definir pelo grupo em client_stub-private.h
 */
struct rtable_t;

/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtable_t *rtable_connect(const char *address_port) {

  if(address_port == NULL){
      return NULL;
  }

  struct rtable_t *rtable = (struct rtable_t *) malloc(sizeof(struct rtable_t));
  struct sockaddr_in *server = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));

  if(rtable == NULL || server == NULL){
      return NULL;
  }

  rtable->server = server;

  char* address = strdup(address_port);

  char* hostname = strdup(strtok(address, ":"));
  rtable->host = hostname;
  char* port = strtok(NULL, "");

  rtable->server->sin_family = AF_INET;
  rtable->server->sin_port = htons(atoi(port));

  if((network_connect(rtable)) == -1){
      free(rtable);
      free(hostname);
      free(address);
      free(server);
      return NULL;
  }

  free(hostname);
  free(address);
  free(server);
  printf("Ligação ao servidor estabelecida\n");
  return rtable;

}


/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtable_disconnect(struct rtable_t *rtable) {

  if(rtable == NULL)return -1;

  if(network_close(rtable)<0)return -1;

  free(rtable);
  return 0;
}

/* Função para adicionar um elemento na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtable_put(struct rtable_t *rtable, struct entry_t *entry) {

  int res;
  struct MessageT* msg;

  msg = entry_to_message(entry);
  if(msg == NULL) return -1;

  msg = network_send_receive(rtable, msg);

  if(msg == NULL) return -1;

  if(msg->opcode == 98) {
    printf("Não é possível executar operações de escrita\n");
  }

  res = ((msg->opcode - OP_PUT) == 1) ? 0 : -1;

  message_t__free_unpacked(msg, NULL);

  return res;

}

/* Função para obter um elemento da tabela.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtable_get(struct rtable_t *rtable, char *key) {

  struct MessageT* msg;
  struct data_t * data;

  msg = key_to_message(key, OP_GET);
  if(msg == NULL) {
    perror("mensagem null");
    return NULL;
  }

  msg = network_send_receive(rtable, msg);

  if(msg == NULL) {
    perror("mensagem null");
    return NULL;
  }

  if((msg->opcode - OP_GET) != 1) {
    return NULL;
  }

  char* value = NULL;
  if(msg->data_size > 0) {
      value = strdup(msg->data);
  }

  int size = msg->data_size;

  data = data_create2(size, value);

  if(data == NULL) return NULL;

  message_t__free_unpacked(msg, NULL);

  return data;

}

/* Função para remover um elemento da tabela. Vai libertar
 * toda a memoria alocada na respetiva operação rtable_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtable_del(struct rtable_t *rtable, char *key) {

  int res;
  struct MessageT* msg;

  msg = key_to_message(key, OP_DEL);
  if(msg == NULL) return -1;

  msg = network_send_receive(rtable, msg);

  if(msg == NULL) return -1;

  if(msg->opcode == 98) {
    printf("Não é possível executar operações de escrita\n");
  }

  res = ((msg->opcode - OP_DEL) == 1) ? 0 : -1;

  message_t__free_unpacked(msg, NULL);

  return res;

}

/* Devolve o número de elementos contidos na tabela.
 */
int rtable_size(struct rtable_t *rtable) {

  int res;
  struct MessageT* msg;

  msg = init_message(OP_SIZE, CT_NONE);
  if(msg == NULL) return -1;

  msg = network_send_receive(rtable, msg);

  if(msg == NULL) return -1;

  res = ((msg->opcode - OP_SIZE) == 1) ? 0 : -1;
  res = msg->result;

  message_t__free_unpacked(msg, NULL);

  return res;

}

/* Devolve um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento a NULL.
 */
char **rtable_get_keys(struct rtable_t *rtable) {
  char** keys;
  MessageT* msg;

  msg = init_message(OP_GETKEYS, CT_NONE);
  if(msg == NULL) return NULL;

  msg = network_send_receive(rtable, msg);

  if((msg->opcode - OP_GETKEYS) != 1) return NULL;

  int size = msg->n_keys;

  if(msg->keys == NULL){
    message_t__free_unpacked(msg, NULL);
    return NULL;
  }

  keys = (char**) malloc(sizeof(char *) * (size+1));
  if(keys == NULL) return NULL;

  for(int i = 0; i < size; i++) {
    keys[i] = strdup(msg->keys[i]);
  }

  keys[size] = NULL;

  message_t__free_unpacked(msg, NULL);

  return keys;

}

/* Liberta a memória alocada por rtable_get_keys().
 */
void rtable_free_keys(char **keys) {
  int i = 0;
  while(keys[i] != NULL) {
    free(keys[i]);
    i++;
  }
  free(keys);
}

/* Função que imprime o conteúdo da tabela remota para o terminal.
 */
void rtable_print(struct rtable_t *rtable) {
  MessageT* msg;

  msg = init_message(OP_PRINT, CT_NONE);

  msg = network_send_receive(rtable, msg);

  if((msg->opcode - OP_PRINT) == 1) {
    int size = msg->n_keys;

    printf("\n          Tabela\n");
    printf("-------------------------------\n");
    for(int i = 0; i < size; i++){
      printf("Chave: %-10s ", msg->keys[i]);
      printf("Valor: %s\n", msg->values[i]);
    }

  }

  message_t__free_unpacked(msg, NULL);
}

/* Função que imprime o conteúdo das estatisticas para o terminal.
 */
struct statistics *rtable_stats(struct rtable_t *rtable) {
  // char** keys;
  MessageT* msg;
  struct statistics* stats_struct = NULL;

  msg = init_message(OP_STATS, CT_NONE);

  msg = network_send_receive(rtable, msg);

  if((msg->opcode - OP_STATS) != 1) return NULL;

  stats_struct = stats_create();

  stats_struct->size_count = msg->stats[0];
  stats_struct->del_count = msg->stats[1];
  stats_struct->get_count = msg->stats[2];
  stats_struct->put_count = msg->stats[3];
  stats_struct->getkeys_count = msg->stats[4];
  stats_struct->table_print_count = msg->stats[5];
  stats_struct->avg_response = msg->avg_response;

  message_t__free_unpacked(msg, NULL);

  return stats_struct;
}
