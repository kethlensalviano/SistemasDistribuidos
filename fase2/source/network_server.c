/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA 
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 2 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835 
*   MANUEL LOPES - 49023
*/

#include "table_skel.h"
#include "inet.h"
#include "message.h"
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

int sockfd;
struct sockaddr_in server, client;

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port) {

        // Cria socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar socket");
        return -1;
    }

    // Preenche estrutura server com endereço(s) para associar (bind) à socket 
    server.sin_family = AF_INET;
    server.sin_port =  htons(port); //htons(atoi(argv[1])); // Porta TCP
    server.sin_addr.s_addr = htonl(INADDR_ANY); // Todos os endereços na máquina

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

    // Faz bind
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao fazer bind");
        close(sockfd);
        return -1;
    }

    // Esta chamada diz ao SO que esta é uma socket para receber pedidos
    if (listen(sockfd, 0) < 0){
        perror("Erro ao executar listen");
        close(sockfd);
        return -1;
    }

    printf("Servidor à espera de dados\n");

    return sockfd;

}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
MessageT *network_receive(int client_socket) {

    MessageT* msg;
    int nbytes = 0, size_msg = 0;

    if((nbytes = read_all(client_socket, &size_msg, sizeof(int))) < 0){
		perror("Erro ao receber tamanho da mensagem");
		close(client_socket);
		return NULL;
	}

    int converted_size = 0;
    converted_size = ntohl(size_msg);
    uint8_t in_buf[converted_size];

	if((nbytes = read_all(client_socket, in_buf, converted_size)) < 0){
		perror("Erro ao receber dados do cliente");
		close(client_socket);
		return NULL;
	}

    if(nbytes == 0) return NULL;

    //msg = (struct MessageT*) malloc(sizeof(struct MessageT));
    msg = message_t__unpack(NULL, nbytes, in_buf);
    if(msg == NULL) return NULL;

    return msg;

}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, MessageT *msg) {

    uint8_t *out_buf;
    int size, nbytes = -1;;

    size = message_t__get_packed_size(msg);
    out_buf = malloc(size);

    message_t__pack(msg, out_buf);

    int size_msg = htonl(size);

    if((nbytes = write_all(client_socket, &size_msg, sizeof(int))) < 0){
		perror("Erro ao enviar tamanho da mensagem");
		close(client_socket);
		return -1;
	}

	if((nbytes = write_all(client_socket, out_buf, size)) != size){
		perror("Erro ao enviar resposta ao cliente");
		close(client_socket);
		return -1;
	}
 
    message_t__free_unpacked(msg, NULL);

    free(out_buf);

    return 0;

}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close() {

    return close(sockfd);

}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */

int network_main_loop(int listening_socket) {

    MessageT* msg;
    socklen_t size_client = sizeof(struct sockaddr);
    int client_socket;

    // accept bloqueia à espera de pedidos de conexão.
    // Quando retorna já foi feito o "three-way handshake" e connsockfd é uma
    // socket pronta a comunicar com o cliente.
    while ((client_socket = accept(listening_socket,(struct sockaddr *) &client, &size_client)) != -1) {

        while((msg = network_receive(client_socket)) != NULL){

            if(invoke(msg) != 0) {
                replace_message(OP_ERROR, CT_NONE, msg);
                perror("erro ao invocar resposta do servidor.");
            }

            network_send(client_socket, msg);
        }
		// Fecha socket referente a esta conexão
        close(client_socket);
        
        printf("cliente desconectado\n");
    }

    return 0;

}
