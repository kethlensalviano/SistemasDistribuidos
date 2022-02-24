OBJ_DIR= object/
SRC_DIR = source/
INC_DIR = include/
BIN_DIR = binary/
LIB_DIR = library/

CC = gcc
LD = ld
CFLAGS = -D THREADED -Wall -pthread -lzookeeper_mt -Wextra -g -Iinclude -I${PROTOC_DIR}include/ -c
LFLAGS = -Iinclude -Wall -pthread -lzookeeper_mt
LDFLAGS = ${PROTOC_DIR}lib/libprotobuf-c.a
COMPILE  = $(CC) $(CFLAGS) -o $(OBJ_DIR)$@ $<

PROTOC_DIR=/usr/local/

CLIENT_LIB = $(OBJ_DIR)data.o $(OBJ_DIR)entry.o $(OBJ_DIR)list.o $(OBJ_DIR)list-private.o $(OBJ_DIR)table.o $(OBJ_DIR)table-private.o $(OBJ_DIR)client_stub.o $(OBJ_DIR)client_stub-private.o $(OBJ_DIR)network_client.o $(OBJ_DIR)message.o $(OBJ_DIR)sdmessage.pb-c.o $(OBJ_DIR)stats-private.o
SERVER_LIB = $(OBJ_DIR)data.o $(OBJ_DIR)entry.o $(OBJ_DIR)list.o $(OBJ_DIR)list-private.o $(OBJ_DIR)table.o $(OBJ_DIR)table-private.o $(OBJ_DIR)client_stub.o $(OBJ_DIR)network_client.o $(OBJ_DIR)table_skel.o $(OBJ_DIR)network_server.o $(OBJ_DIR)stats-private.o $(OBJ_DIR)message.o $(OBJ_DIR)sdmessage.pb-c.o

OBJS = data.o entry.o list.o list-private.o table.o table-private.o sdmessage.pb-c.o network_server.o table_skel.o message.o network_client.o client_stub.o client_stub-private.o table_client_private.o stats-private.o

all: sdmessage.pb-c.c $(OBJS) server-lib.o table-server client-lib.o table-client

########################################

%.o: $(SRC_DIR)%.c
	 $(COMPILE)

sdmessage.pb-c.c:
	${PROTOC_DIR}bin/protoc-c sdmessage.proto --c_out=./
	$ mv sdmessage.pb-c.h $(INC_DIR)
	$ mv sdmessage.pb-c.c $(SRC_DIR)

sdmessage.pb-c.o:
	$(CC) $(CFLAGS) $(SRC_DIR)sdmessage.pb-c.c -o $(OBJ_DIR)sdmessage.pb-c.o

client-lib.o:
	$(LD) -r $(CLIENT_LIB) -o $(LIB_DIR)client-lib.o

server-lib.o:
	$(LD) -r $(SERVER_LIB) -o $(LIB_DIR)server-lib.o

#########################

table-client:
	$(CC)  -D THREADED $(LIB_DIR)client-lib.o $(OBJ_DIR)table_client_private.o $(SRC_DIR)table_client.c -o $(BIN_DIR)table-client $(LDFLAGS) $(LFLAGS)

table-server:
	$(CC)  $(LIB_DIR)server-lib.o $(SRC_DIR)table_server.c -o $(BIN_DIR)table-server $(LDFLAGS) $(LFLAGS)

#########################

clean:
	rm -f $(BIN_DIR)* $(LIB_DIR)*.o $(OBJ_DIR)*.o $(INC_DIR)*pb-c.h $(SRC_DIR)*pb-c.c
