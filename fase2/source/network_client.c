/*FACULDADE DE CIENCIAS DA UNIVERSIDADE DE LISBOA
*   DEPARTAMENTO DE INFORMÁTICA 
*   DISCIPLINA DE SISTEMAS DISTRIBUIDOS
*   FASE 2 DO PROJETO
*   GRUPO 12
*   DAVID MAROUCO - 53509
*   KETHLEN SALVIANO - 51835 
*   MANUEL LOPES - 49023
*/

#include "client_stub.h"
#include "client_stub-private.h"
#include "sdmessage.pb-c.h"
#include "network_client.h"
#include "inet.h"
#include "message.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtable;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtable;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtable_t *rtable) {

    if((rtable->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP");
        return -1;
    }

    if(inet_pton(AF_INET, rtable->host, &rtable->server->sin_addr) < 1) {
        perror("Erro ao converter IP\n");
        close(rtable->sockfd);
        return -1;
    }

    if(connect(rtable->sockfd, (struct sockaddr *)rtable->server, sizeof(*rtable->server)) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(rtable->sockfd);
        return -1;
    }
    
    return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtable_t;
 * - Reservar memória para serializar a mensagem contida em msg;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Libertar a memória ocupada pela mensagem serializada enviada;
 * - Esperar a resposta do servidor;
 * - Reservar a memória para a mensagem serializada recebida;
 * - De-serializar a mensagem de resposta, reservando a memória 
 *   necessária para a estrutura message_t que é devolvida;
 * - Libertar a memória ocupada pela mensagem serializada recebida;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
MessageT *network_send_receive(struct rtable_t * rtable, MessageT *msg) {

    //uint8_t in_buf[MAX_MSG];
    uint8_t *out_buf;
    int size_rmsg;

    int nbytes, size_msg;
    size_msg = message_t__get_packed_size(msg);
    out_buf = malloc(size_msg);
    message_t__pack(msg, out_buf);

    int converted_out = htonl(size_msg);

    if((nbytes = write_all(rtable->sockfd, &converted_out, sizeof(int))) < 0){
		perror("Erro ao receber tamanho da mensagem");
		close(rtable->sockfd);
		return NULL;
	}

    if((nbytes = write_all(rtable->sockfd, out_buf, size_msg)) != size_msg) {
        perror("Erro ao enviar mensagem ao servidor");
        close(rtable->sockfd);
        return NULL;
    }

    message_t__free_unpacked(msg, NULL);
    //printf("À espera de resposta do servidor ...\n");

    if((size_rmsg = read_all(rtable->sockfd, &size_msg, sizeof(int))) < 0) {
        perror("Erro ao receber int do servidor");
        close(rtable->sockfd);
        return NULL;
    }

    int converted_in = ntohl(size_msg);
    uint8_t in_buf[converted_in];

    if((size_rmsg = read_all(rtable->sockfd, in_buf, converted_in)) != converted_in) {
        perror("Erro ao receber mensagem do servidor");
        close(rtable->sockfd);
        return NULL;
    }

    msg = message_t__unpack(NULL, size_rmsg, in_buf);
    if(msg == NULL) return NULL;

    free(out_buf);
    return msg;

}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtable_t * rtable) {
    return close(rtable->sockfd);
}
