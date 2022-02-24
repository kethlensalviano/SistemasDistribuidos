/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 2 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <regex.h>

#include "data.h"
#include "entry.h"
#include "client_stub.h"
#include "message.h"

int test_input(int argc, char** argv){
    if (argc != 2){
        printf("Uso: ./table_client <ip_servidor>:<porto_servidor>\n");
        printf("Exemplo de uso: ./table_client 127.0.0.1:12345\n");
        return -1;
    }

    char address_port[25];
    strncpy(address_port, argv[1], 24);
    address_port[24] = '\0';

    regex_t regex;
    int value;

    if((value = regcomp(&regex, "[^()]:[^()]", 0)) == 0) {
      value = regexec(&regex, address_port, 0, NULL, 0);
      if(value != 0) {
        printf("Uso: ./table_client <ip_servidor>:<porto_servidor>\n");
        printf("Exemplo de uso: ./table_client 127.0.0.1:12345\n");
        return -1;
      }
    }

    regfree(&regex);
    return 0;
}

extern struct rtable_t *client_rtable;

void signal_handler() {
    printf("servidor desconectado\n");
    if (rtable_disconnect(client_rtable) != 0) {
      perror("Erro ao desconectar-se do servidor");
    }
    exit(1);
}

int get_opcode(char* operacao){
    int opcode = 0;
    if(strcmp(operacao, "size") == 0){
        opcode = OP_SIZE;
    }else if(strcmp(operacao, "get") == 0){
        opcode = OP_GET;
    }else if(strcmp(operacao, "put") == 0){
        opcode = OP_PUT;
    }else if(strcmp(operacao, "del") == 0){
        opcode = OP_DEL;
    }else if(strcmp(operacao, "getkeys") == 0){
        opcode = OP_GETKEYS;
    }else if(strcmp(operacao, "table_print") == 0){
        opcode = OP_PRINT;
    } else {
         opcode = -1;
    }
    return opcode;

}

int exec_command(char *str,struct rtable_t *client_rtable){

    // verifica se str eh string vazia
    if (str[0] == '\0') return -1;

    char *c = strtok(str, " ");
    char *key;
    char *data;

    if (strcmp(c, "quit") == 0){
      return 0;
    }

    int opcode = get_opcode(c);

    switch (opcode) {

      case OP_SIZE:;
          int size = rtable_size(client_rtable);
          printf("Tabela tem %d elementos\n", size);
          break;
      
      case OP_GET:
          key = strtok(NULL, " ");

          if (key == NULL){
            printf("Input invalido. Uso: get <key>\n");
            break;
          }

          struct data_t *d = rtable_get(client_rtable,key);

          if(d->data == NULL) {
            printf("key %s não existe\n", key);
            data_destroy(d);
            return 0;
          }

          printf("key: %s value: %s\n", key, (char*) d->data);

          data_destroy(d);
          break;

      case OP_PUT:
          key = strtok(NULL, " ");
          data = strtok(NULL, "");

          if (key == NULL || data == NULL){
            printf("Input invalido. Uso: put <key> <data>\n");
            break;
          }


          struct data_t *data_struct = data_create2 (strlen(data)+1, data);
          struct entry_t *entry = entry_create(key, data_struct);

          if (rtable_put(client_rtable, entry) != 0){
            printf("Erro ao tentar introduzir a entry %s\n",key);
          }else{
            printf("Entry %s foi introduzida\n",key);
          }

          free(data_struct);
          free(entry);
          break;

      case OP_DEL:
          key = strtok(NULL, " ");

          if (key == NULL){
            printf("Input invalido. Uso: del <key>\n");
            break;
          }

          if (rtable_del(client_rtable,key) != 0){
            printf("Erro ao tentar apagar a entry %s\n",key);
          }else{
            printf("Entry %s foi apagada\n",key);
          }

          break;
      
      case OP_GETKEYS:;
          char **keys = rtable_get_keys(client_rtable);
          if(keys == NULL) {
            perror("Não há keys");
            return -1;
          }

          int i = 0;
          printf("Lista das chaves:\n");
          while (keys[i] != NULL) {
            printf("%s\n", keys[i]);
            i++;
          }

          rtable_free_keys(keys);
          break;

      case OP_PRINT:
          rtable_print(client_rtable);
          break;
      
      default:
          break;
    }

    printf("-------------------------------\n");
    return 0;
}
