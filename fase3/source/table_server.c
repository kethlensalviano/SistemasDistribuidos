/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 3 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835
*   MANUEL LOPES - 49023
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "table_skel.h"
#include "network_server.h"

int testInput(int argc){
    if (argc != 3){
	    printf("Uso: ./table_server <server_port> <n_lists>\n");
        printf("Exemplo de uso: ./table_server 12345 5\n");
        return -1;
    }
    return 0;
}

void signal_handler() {
    printf("servidor desconectado\n");
    if (network_server_close() != 0) {
      perror("Erro ao desconectar o servidor");
    }
    table_skel_destroy();
    exit(1);
}

int main(int argc, char **argv) {

    int port, listening_socket, n_lists;

    /* Testar os argumentos de entrada */
    if (testInput(argc) < 0) return -1;

    if(atoi(argv[2]) == 0) return -1;

    port = atoi(argv[1]);
    n_lists = atoi(argv[2]);

    /* inicialização da camada de rede */
    if((listening_socket = network_server_init(port)) < 0) return -1;

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;
    if(sigaction(SIGINT, &sa, NULL) == -1 ){
      perror("sigaction");
      return -1;
    }

    if(table_skel_init(n_lists) < 0) {
        return -1;
    }

    int result = network_main_loop(listening_socket);

    if(network_server_close() < 0) {
        perror("erro ao desconectar o servidor");
    }

    table_skel_destroy();
    return result;

}
