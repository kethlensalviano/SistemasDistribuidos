/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 3 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/

#ifndef _TABLE_SKEL_PRIVATE_H
#define _TABLE_SKEL_PRIVATE_H

#include "sdmessage.pb-c.h"
#include "table.h"

int write_mutex(MessageT *msg);

int read_mutex(MessageT *msg);

int screening(MessageT *msg);

int actions(MessageT *msg);

int write_mutex_stats(int opcode,double resp_time);

int read_mutex_stats(MessageT *msg);

#endif
