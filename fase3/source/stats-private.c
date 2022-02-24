/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 3 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/

#include <stdlib.h>
#include <stdio.h>

#include "stats-private.h"
#include "message.h"

/* Função que cria uma nova estrutura de estatisticas, iniciada com todos
*  os valores a zero
*/
struct statistics *stats_create(){

  struct statistics * stats_struct;
  stats_struct = (struct statistics *) malloc(sizeof (struct statistics));

  if(stats_struct != NULL) {
    stats_struct->size_count = 0;
    stats_struct->del_count = 0;
    stats_struct->get_count = 0;
    stats_struct->put_count = 0;
    stats_struct->getkeys_count = 0;
    stats_struct->table_print_count = 0;
    stats_struct->avg_response = 0.0;
  }
  return stats_struct;
}

/* Função liberta a memoria da estrutura statistics
*/
void stats_destroy(struct statistics *stats_struct){

  free(stats_struct);

}

/* Incrementa o contador da operacao com opcode corresponmdente
*/
int stats_increment(int opcode,struct statistics* stats_struct){

  int res = -1;
  switch(opcode) {
      case OP_SIZE+1:
        stats_struct->size_count++;
        res = 0;
        break;

      case OP_DEL+1:
        stats_struct->del_count++;
        res = 0;
        break;

      case OP_GET+1:
        stats_struct->get_count++;
        res = 0;
        break;

      case OP_PUT+1:
        stats_struct->put_count++;
        res = 0;
        break;

      case OP_GETKEYS+1:
        stats_struct->getkeys_count++;
        res = 0;
        break;

      case OP_PRINT+1:
        stats_struct->table_print_count++;
        res = 0;
        break;

      default:
        break;
  }
  return res;
}

/* Recalcula a média de tempo de resposta
*/
int stats_recalc_avg(struct statistics* stats_struct, double resp_time){
  int sum_ops = stats_struct->size_count;
  sum_ops += stats_struct->del_count;
  sum_ops += stats_struct->get_count;
  sum_ops += stats_struct->put_count;
  sum_ops += stats_struct->getkeys_count;
  sum_ops += stats_struct->table_print_count;

  double total_ops = (double) sum_ops;
  double old_avg = stats_struct->avg_response;
  double new_avg = old_avg + ((resp_time - old_avg) / total_ops);

  stats_struct->avg_response = new_avg;
  return 0;
}

/* Faz print da estrutura
*/
void stats_print(struct statistics* stats_struct){
  printf("\n          Estatiscas\n");
  printf("-------------------------------\n");
  printf("Nº de operações size:        %d\n",stats_struct->size_count);
  printf("Nº de operações del:         %d\n",stats_struct->del_count);
  printf("Nº de operações get:         %d\n",stats_struct->get_count);
  printf("Nº de operações put:         %d\n",stats_struct->put_count);
  printf("Nº de operações getkeys:     %d\n",stats_struct->getkeys_count);
  printf("Nº de operações table_print: %d\n",stats_struct->table_print_count);
  printf("Tempo médio de resposta:     %.3f milisegundos\n",(float) stats_struct->avg_response);
}
