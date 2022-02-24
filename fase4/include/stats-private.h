/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 3 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/

#ifndef _STATS_H
#define _STATS_H /* Módulo statistics */

/* Estrutura que define as estatisticas.*/

struct statistics {
  int size_count;           /* Contagem de operaçoes size */
  int del_count;            /* Contagem de operaçoes del */
  int get_count;            /* Contagem de operaçoes get */
  int put_count;            /* Contagem de operaçoes put */
  int getkeys_count;        /* Contagem de operaçoes getkeys */
  int table_print_count;    /* Contagem de operaçoes table_print */
  double avg_response;       /* Valor do tempo médio de resposta do servidor */
};


/* Função que cria uma nova estrutura de estatisticas, iniciada com todos
*  os valores a zero
*/
struct statistics *stats_create();

/* Função liberta a memoria da estrutura statistics
*/
void stats_destroy(struct statistics *stats_struct);

/* Incrementa o contador da operacao com opcode corresponmdente
*/
int stats_increment(int opcode,struct statistics* stats_struct);

/* Recalcula a média de tempo de resposta
*/
int stats_recalc_avg(struct statistics* stats_struct, double resp_time);

void stats_print(struct statistics* stats_struct);

#endif
