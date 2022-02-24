/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 3 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/

#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "data.h"
#include "entry.h"
#include <zookeeper/zookeeper.h>


/* Remote table, que deve conter as informações necessárias para estabelecer a comunicação com o servidor. A definir pelo grupo em client_stub-private.h
 */
struct rtable_t {
    struct sockaddr_in* server;
    int sockfd;
    char* host;
    zhandle_t *zh;
};

#endif
