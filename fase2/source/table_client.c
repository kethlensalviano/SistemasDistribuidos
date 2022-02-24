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

#include "data.h"
#include "entry.h"
#include "client_stub.h"
#include "message.h"
#include "table_client_private.h"

struct rtable_t *client_rtable;

int main(int argc, char **argv){

    // Verifica se foi passado algum argumento
    if (test_input(argc, argv) < 0) return -1;

    char address_port[25];
    char command[200] = "";

    char *p;

    // Copia os primeiros bytes da string passada como argumento
    strncpy(address_port, argv[1], 24);
    // Garante que a string tem terminação.
    address_port[24] = '\0';

    // Estabelece conexão com o servidor
    client_rtable = rtable_connect(address_port);

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if(sigaction(SIGPIPE, &sa, NULL) == -1 ){
      perror("sigaction");
      return -1;
    }

    //signal(SIGPIPE, signal_handler);

    if (client_rtable == NULL) {
        perror("Erro ao conectar-se ao servidor (cliente)");
        return -1;
    }

    while (strcmp(command,"quit") != 0) {
        printf("\nAções disponíveis: \n\tsize \n\tdel <key> \n\tget <key> \n\tput <key> <data> \n\tgetkeys \n\ttable_print \n\tquit\n\n");
        printf("Insira uma ação: ");

        //le comando introduzido pelo utilizador
        fgets(command,sizeof(command),stdin);

        //remove /n da linha lida por fgets
        if ((p = strchr(command, '\n')) != NULL) *p = '\0';

        if (exec_command(command,client_rtable) != 0) {
           //fputs ("error\n\n", stderr);
        }
    }

    // Desconecta do servidor
    if (rtable_disconnect(client_rtable) != 0) {
        perror("Erro ao desconectar-se do servidor");
        return -1;
    }

    return 0; // Termina
}
