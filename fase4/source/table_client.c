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
#include <unistd.h>
#include <zookeeper/zookeeper.h>

#include "data.h"
#include "entry.h"
#include "client_stub.h"
#include "message.h"
#include "table_client_private.h"

struct rtable_t *client_rtable;

static zhandle_t *zh;
static int is_connected;
const char *kvstore = "/kvstore";
const char *primary_path = "/kvstore/primary";
typedef struct String_vector zoo_string;
zoo_string* children_list =	NULL;
static char *watcher_ctx = "ZooKeeper Data Watcher"; //o que isto faz?

/**
* Watcher function for connection state change events
*/
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1;
		} else {
			is_connected = 0;
		}
	}
}

static void child_watcher_client(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	//int zoo_data_len = ZDATALEN;

	if (state == ZOO_CONNECTED_STATE) {
		if (type == ZOO_CHILD_EVENT) {

			// printf("\nwatch cliente\n");
			// Se saiu o servidor primário, fechar a ligação que existia ao servidor,
			// e atualizar a informação armazenada na estrutura rtable_t;  (falta)
			// informar o utilizador sobre a impossibilidade de executar operações na tabela;
			// voltar a ativar o watch;
			if(ZNONODE == zoo_exists(zh, primary_path, 0, NULL)) {
				if (rtable_disconnect(client_rtable) != 0) {
					perror("Erro ao desconectar-se do servidor");
				}
				client_rtable = NULL;

				//if (ZOK != zoo_wget_children(zh, kvstore, child_watcher_client, watcher_ctx, children_list)) {
				//	printf("Error setting watch\n");
				//}
				printf("\n\nServidor primário desconectado, aguarde nova conexão\n");
				//sleep(5);
				// Se entrou um servidor primário, atualizar a informação relativamente
				// ao IP:porto do servidor, iniciar uma nova ligação, e voltar a permitir
				// a execução de operações na tabela; voltar a ativar o watch;
			} else if(ZNONODE != zoo_exists(zh, primary_path, 0, NULL) && client_rtable == NULL) {
				char address_port[1024];
				int address_len = sizeof(address_port);
				if (ZOK == zoo_get(zh, primary_path, 0, address_port, &address_len, NULL)) {

					if ((client_rtable = rtable_connect(address_port)) != NULL){
						printf("\nAções disponíveis: \n\tsize \n\tdel <key> \n\tget <key> \n\tput <key> <data> \n\tgetkeys \n\ttable_print \n\tstats \n\tquit\n\n");
						printf("Insira uma ação: \n");
					}
				}

				if (client_rtable == NULL){
					printf("Erro a restabelecer ligação!\n");
					zookeeper_close(zh);
					exit(-1);
				}
			}
			if (ZOK != zoo_wget_children(zh, kvstore, child_watcher_client, watcher_ctx, children_list)) {
				printf("Error setting watch\n");
			}

		}
	}
}

int zookeeper_init_client(const char* address_zk) {

	if(address_zk == NULL){
		return -1;
	}

	zh = zookeeper_init(address_zk, connection_watcher, 2000, 0, NULL, 0);
	if (zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
		return -1;
	}

	sleep(3); /* Sleep a little for connection to complete */
	if (is_connected == 1) {

		// obter e fazer watch aos filhos de kvstore
		children_list =	(zoo_string *) malloc(sizeof(zoo_string));

		if (ZOK != zoo_wget_children(zh, kvstore, child_watcher_client, watcher_ctx, children_list)) {
			printf("Error setting watch\n");
			return -1;
		}

		// existe primary
		if(ZNONODE != zoo_exists(zh, primary_path, 0, NULL)) {

			char address_port[1024];
			int address_len = sizeof(address_port);
			if (ZOK == zoo_get(zh, primary_path, 0, address_port, &address_len, NULL)) {
				while ((client_rtable = rtable_connect(address_port)) == NULL){
					sleep(1);
				}
			}
			return 0;
		}
	}
	deallocate_String_vector(children_list);
	zookeeper_close(zh);
	printf("\nNão foi encontrado servidor em %s\n",address_zk);
	return -1;
}

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
	//client_rtable = get_rtable();

	if(zookeeper_init_client(address_port) != 0){
		return -1;
	}

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
		printf("\nAções disponíveis: \n\tsize \n\tdel <key> \n\tget <key> \n\tput <key> <data> \n\tgetkeys \n\ttable_print \n\tstats \n\tquit\n\n");
		printf("Insira uma ação: ");

		//le comando introduzido pelo utilizador
		fgets(command,sizeof(command),stdin);

		//remove /n da linha lida por fgets
		if ((p = strchr(command, '\n')) != NULL) *p = '\0';

		if (exec_command(command,client_rtable) != 0) {
			printf("Erro a executar comando");
		}
	}

	// Desconecta do servidor
	if (rtable_disconnect(client_rtable) != 0) {
		perror("Erro ao desconectar-se do servidor");
		return -1;
	}
	deallocate_String_vector(children_list);
	zookeeper_close(zh);
	return 0; // Termina
}
