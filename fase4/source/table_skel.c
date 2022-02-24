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
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <zookeeper/zookeeper.h>

#include "sdmessage.pb-c.h"
#include "table.h"
#include "message.h"
#include "data.h"
#include "entry.h"
#include "table_skel_private.h"
#include "stats-private.h"
#include "client_stub.h"

struct table_t *table;
struct statistics *stats_struct;
extern int write_thread;
extern int write_stats;

pthread_mutex_t mutex_read = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_write = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_read_stats = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_write_stats = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_read = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_write = PTHREAD_COND_INITIALIZER;

pthread_cond_t cond_read_stats = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_write_stats = PTHREAD_COND_INITIALIZER;

static zhandle_t *zh;
static int is_connected;
const char *kvstore = "/kvstore";
const char *primary_path = "/kvstore/primary";
const char *backup_path = "/kvstore/backup";
const char *root_path = "/";
const char *host_port;
int size_host_port;

typedef struct String_vector zoo_string;
zoo_string* children_list =	NULL;
static char *watcher_ctx = "ZooKeeper Data Watcher"; //o que isto faz?
int isprimary = 0;
struct rtable_t *rtable_server;

/**
* Watcher function for connection state change events
*/
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {

	//to remove warnings
	if (zzh == NULL || path == NULL || context == NULL){
		//do nothing
	}

	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1;
		} else {
			is_connected = 0;
		}
	}
}

static void child_watcher_server(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
  zoo_string* children_list =	(zoo_string *) malloc(sizeof(zoo_string));
	// int zoo_data_len = ZDATALEN;

	//to remove warnings
	if (wzh == NULL || zpath == NULL){
		//do nothing
	}

	if (state == ZOO_CONNECTED_STATE) {
		if (type == ZOO_CHILD_EVENT) {
			// printf("Mudanca na kvstore!!\n");
	      //Se for servidor primário e o backup tiver saído, não aceita mais pedidos
	      //de escrita dos clientes até que volte a haver backup. Volta a ativar o watch.
	      if(isprimary) {
          if(ZNONODE == zoo_exists(zh, backup_path, 0, NULL) && rtable_server != NULL) {
              rtable_disconnect(rtable_server);
              rtable_server = NULL;
          } else {

            //Se for servidor primário e houve ativação do servidor de backup, guarda o
            //seu par IP:porta, estabelece ligação, transfere todas as entradas da tabela
            //para o servidor de backup (realizando uma sequência de operações put), e
            //volta a aceitar pedidos de escrita dos clientes. Volta a ativar o watch.
            char address_port[25];
            int address_len = sizeof(address_port);
            if (ZOK == zoo_get(zh, backup_path, 0, address_port, &address_len, NULL)) {
								address_port[24] = '\0';

              	// conectar com o backup e update table utilizando o client_stub
                while ((rtable_server = rtable_connect(address_port)) == NULL){
									sleep(1);
								}

                char** keys = table_get_keys(table);
                struct entry_t *entry;
                struct data_t *data;
								int i = 0;

                while(keys[i] != NULL){
                  data = table_get(table, keys[i]);
                  entry = entry_create(keys[i], data);
									if (rtable_put(rtable_server, entry) != 0){
										printf("Erro ao tentar introduzir a entry %s no servidor backup\n",keys[i]);
									}else{
										printf("Entry %s foi introduzida no servidor backup\n",keys[i]);
									}
                  entry_destroy(entry);
                  i++;
                }
              }
            }
	        }else{
						//Se for servidor backup e o primário tiver saído, autopromove-se a servidor
						//primário. Volta a ativar o watch.
						if(ZNONODE == zoo_exists(zh, primary_path, 0, NULL)) {
							isprimary = 1;
							if (ZOK == zoo_create(zh, primary_path, host_port, size_host_port, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0)) {
								if (ZOK == zoo_delete(zh, backup_path, 0)) {
									zoo_wget_children(zh, kvstore, child_watcher_server, watcher_ctx, children_list);
								}
								printf("Promovido a Primary\n");
							}
						}
					}

				/* Get the updated children and reset the watch */
	 			if (ZOK != zoo_wget_children(zh, kvstore, child_watcher_server, watcher_ctx, children_list)) {
	 				printf("Error setting watch\n");
	 			}
      }
    }
	free(children_list);
}

int zookeeper_init_server(const char* host_port_server, const char* host_port_zk) {

    size_host_port = strlen(host_port_server) + 1;
    host_port = host_port_server;

    // conectar ao zookeeper com host port zookeeper
    zh = zookeeper_init(host_port_zk, connection_watcher,	2000, 0, NULL, 0);
	if (zh == NULL)	{
		fprintf(stderr, "Error connecting to ZooKeeper server!\n");
	}

    sleep(3); /* Sleep a little for connection to complete */
    if (is_connected) {
	    // se kvstore não existe
			if (ZNONODE == zoo_exists(zh, kvstore, 0, NULL)) {
				printf("key store não existe\n");

	      // criar kvstore
				if (ZOK == zoo_create( zh, kvstore, NULL, -1, & ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)) {
					printf("key store criada\n");
				} else {
					printf("erro ao criar key store\n");
					return -1;
				}

	      // criar node efemero primary
				if (ZOK != zoo_create(zh, primary_path, host_port, size_host_port, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0)) {
					printf("erro ao criar primary\n");
				}
				isprimary = 1;
			} else {

	      // existe primary e backup - terminar
	      if (ZNONODE != zoo_exists(zh, primary_path, 0, NULL) && ZNONODE != zoo_exists(zh, backup_path, 0, NULL)) {
						deallocate_String_vector(children_list);
						zookeeper_close(zh);
						printf("Ja existe servidor Primario e Backup\n");
						sleep(2);
	          return -1;

	      // nao existe primary e nao existe backup - criar primary efemero
				} else if(ZNONODE == zoo_exists(zh, primary_path, 0, NULL) && ZNONODE == zoo_exists(zh, backup_path, 0, NULL)) {
	          // criar node efemero primary
	          if (ZOK != zoo_create(zh, primary_path, host_port, size_host_port, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0)) {
	              printf("erro ao criar primary\n");
							}
						printf("Ligado como Primary\n");
	          isprimary = 1;

	      // existe primary e nao existe backup - criar backup efemero
				} else if (ZNONODE != zoo_exists(zh, primary_path, 0, NULL) && ZNONODE == zoo_exists(zh, backup_path, 0, NULL)) {
		        // criar node efemero backup
		        if (ZOK != zoo_create(zh, backup_path, host_port, size_host_port, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0)) {
		            printf("erro ao criar backup\n");
							}
						printf("Ligado como Backup\n");
		        isprimary = 0;

	      // nao existe primary e existe backup - esperar backup se tornar primary e tentar de novo
				} else if (ZNONODE == zoo_exists(zh, primary_path, 0, NULL) && ZNONODE != zoo_exists(zh, backup_path, 0, NULL)) {
						sleep(5);
		        if(ZNONODE == zoo_exists(zh, primary_path, 0, NULL) && ZNONODE != zoo_exists(zh, backup_path, 0, NULL)) {
		          if (ZOK != zoo_create(zh, backup_path, host_port, size_host_port, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0)) {
		              printf("erro ao criar backup\n");
								}
								isprimary = 0;
					}
	      }
	    }
      // obter e fazer watch aos filhos de kvstore
      children_list =	(zoo_string *) malloc(sizeof(zoo_string));

      if (ZOK != zoo_wget_children(zh, kvstore, &child_watcher_server, watcher_ctx, children_list)) {
				printf("Error setting watch\n");
			}
      return 0;
    }

    return -1;

}



/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists) {

    if (n_lists > 0) {
        table = table_create(n_lists);
        stats_struct = stats_create();
        return 0;
    }

    return -1;

}

/* Liberta toda a memória e recursos alocados pela função table_skel_init.
 */
void table_skel_destroy() {
    table_destroy(table);
    stats_destroy(stats_struct);

}

int actions(MessageT *msg) {

    int res = 0;
    int size;
    struct data_t *data;
    struct entry_t *entry;
    char** keys;

    switch(msg->opcode) {
        case OP_SIZE:
            size = table_size(table);
            if((res = replace_message(OP_SIZE+1, CT_RESULT, msg)) == 0) {
                msg->result = size;
            }
            break;

        case OP_DEL:
            if(isprimary && rtable_server == NULL) { //nao existe backup
                res = replace_message(OP_ERROR_ZK, CT_NONE, msg);
			} else {

				if(isprimary == 0) {  // backup
                    if(table_del(table, msg->key) == 0) {
                        res = replace_message(OP_DEL+1, CT_NONE, msg);
                    } else {
                        res = replace_message(OP_ERROR, CT_NONE, msg);
                    }
                } else if(isprimary && rtable_server != NULL) { //primary e backup existe
                    if(rtable_del(rtable_server, msg->key) == 0){
                        if(table_del(table, msg->key) == 0) {
                            res = replace_message(OP_DEL+1, CT_NONE, msg);
                        } else {
                            res = replace_message(OP_ERROR, CT_NONE, msg);
                        }
                    } else {
                        res = replace_message(OP_ERROR, CT_NONE, msg);
                    }
                }

            }
            break;

        case OP_GET:
            data = table_get(table, msg->key);
            res = replace_message(OP_GET+1, CT_VALUE, msg);

            if(data != NULL) {
                res = data_to_message(data, msg);
            } else {
                data = data_create(0);
                res = data_to_message(data, msg);
            }
            data_destroy(data);
            break;

        case OP_PUT:
            if(isprimary && rtable_server == NULL) { //nao existe backup
                res = replace_message(OP_ERROR_ZK, CT_NONE, msg);
            } else {
				entry = message_to_entry(msg);

				if(isprimary == 0){ // backup
                    if(table_put(table, entry->key, entry->value) == 0) {
                        res = replace_message(OP_PUT+1, CT_NONE, msg);
                    } else {
                        res = replace_message(OP_ERROR, CT_NONE, msg);
                    }
                } else if(isprimary && rtable_server != NULL) { //primary e backup existe
                    if(rtable_put(rtable_server, entry) == 0){
                        if(table_put(table, entry->key, entry->value) == 0) {
                            res = replace_message(OP_PUT+1, CT_NONE, msg);
                        } else {
                            res = replace_message(OP_ERROR, CT_NONE, msg);
                        }
                    } else {
                        res = replace_message(OP_ERROR, CT_NONE, msg);
                    }
                } else {
                    res = replace_message(OP_ERROR, CT_NONE, msg);
                }
				entry_destroy(entry);
            }
            break;

        case OP_GETKEYS:
            keys = table_get_keys(table);
            if(keys != NULL){
                res = replace_message(OP_GETKEYS+1, CT_KEYS, msg);
            } else {
                res = replace_message(OP_ERROR, CT_NONE, msg);
            }

            res = keys_to_message(msg, keys);

            break;

        case OP_PRINT:
            res = table_to_message(msg,table);
            break;

        case OP_STATS:

            //le estrutura statistics e constroi mensagem
            read_mutex_stats(msg);

            break;

        default:
            break;
    }

    return res;

}

int write_mutex(MessageT *msg) {
    int res = 0;

    pthread_mutex_lock(&mutex_write);

    while(write_thread == 1) {
        pthread_cond_wait(&cond_write, &mutex_write);
    }

    pthread_mutex_unlock(&mutex_write);

    pthread_mutex_lock(&mutex_read);

    write_thread = 1;
    //do some writing
    //sleep(5);
    res = actions(msg);
    write_thread = 0;

    pthread_cond_broadcast(&cond_read);
    pthread_cond_broadcast(&cond_write);

    pthread_mutex_unlock(&mutex_read);
    return res;
}

int read_mutex(MessageT *msg) {
    int res = 0;

    pthread_mutex_lock(&mutex_read);

    while(write_thread == 1) {
        pthread_cond_wait(&cond_read, &mutex_read);
    }

    pthread_mutex_unlock(&mutex_read);

    //do some reading
    //sleep(2);
    res = actions(msg);

    return res;
}

int screening(MessageT *msg) {
    int res = 0;
    if(msg->opcode == OP_PUT || msg->opcode == OP_DEL) {
        res = write_mutex(msg);
    } else {
        res = read_mutex(msg);
    }

    return res;

}

int write_mutex_stats(int opcode,double resp_time) {
    int res = 0;

    pthread_mutex_lock(&mutex_write_stats);

    while(write_thread == 1) {
        pthread_cond_wait(&cond_write_stats, &mutex_write_stats);
    }

    pthread_mutex_unlock(&mutex_write_stats);

    pthread_mutex_lock(&mutex_read_stats);

    write_stats = 1;

    if (stats_increment(opcode,stats_struct) == 0) {
        //sleep(5);
        //printf("resp_time: %f\n",resp_time);
        stats_recalc_avg(stats_struct,resp_time);
    }

    write_stats = 0;

    pthread_cond_broadcast(&cond_read_stats);
    pthread_cond_broadcast(&cond_write_stats);

    pthread_mutex_unlock(&mutex_read_stats);
    return res;
}

int read_mutex_stats(MessageT *msg) {
    int res = 0;

    pthread_mutex_lock(&mutex_read_stats);

    while(write_stats == 1) {
        pthread_cond_wait(&cond_read_stats, &mutex_read_stats);
    }

    pthread_mutex_unlock(&mutex_read_stats);

    //do some reading and construct message
    if((res = stats_to_message(msg,stats_struct)) != 0) {
        //sleep(2);
        res = replace_message(OP_ERROR, CT_NONE, msg);
    }

    return res;
}

/* Executa uma operação na tabela (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, tabela nao incializada)
*/
int invoke(MessageT *msg) {

    int res;

    if(msg == NULL) {
        printf("Msg veio Null\n");
        return -1;
    }

    if(table == NULL) {
        printf("table veio Null\n");
        return -1;
    }

    // triagem do tipo de pedido para saber se chama o write_mutex ou read_mutex
    // write -> put, del
    // read -> size, get, getkeys, print

    if ((res = screening(msg)) != 0) {
      printf("Res veio !=0 do screening\n");
      return res;
    }

    return res;

}
