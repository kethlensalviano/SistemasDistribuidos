OBJ_DIR= object/
SRC_DIR = source/
INC_DIR = include/
BIN_DIR = binary/

CC = gcc
CFLAGS = -g -Wall -Iinclude -Wextra -c
LFLAGS = -Iinclude -Wall

DEPS_test_data = $(OBJ_DIR)data.o
DEPS_test_entry = $(DEPS_test_data) $(OBJ_DIR)entry.o
DEPS_test_list = $(DEPS_test_entry) $(OBJ_DIR)list.o $(OBJ_DIR)list-private.o
DEPS_test_table = $(DEPS_test_list) $(OBJ_DIR)table.o $(OBJ_DIR)table-private.o
DEPS_test_serialization = $(DEPS_test_entry) $(OBJ_DIR)serialization.o

all: data.o entry.o list-private.o list.o table-private.o table.o serialization.o test_data test_entry test_list test_table test_serialization

data.o: $(INC_DIR)data.h
	$(CC) $(CFLAGS) $(SRC_DIR)data.c -o $(OBJ_DIR)data.o

entry.o: $(INC_DIR)entry.h
	$(CC) $(CFLAGS) $(SRC_DIR)entry.c -o $(OBJ_DIR)entry.o

list-private.o: $(INC_DIR)list.h $(INC_DIR)list-private.h $(INC_DIR)entry.h
	$(CC) $(CFLAGS) $(SRC_DIR)list-private.c -o $(OBJ_DIR)list-private.o

list.o: $(INC_DIR)list.h $(INC_DIR)list-private.h $(INC_DIR)entry.h
	$(CC) $(CFLAGS) $(SRC_DIR)list.c -o $(OBJ_DIR)list.o

table-private.o: $(INC_DIR)table.h $(INC_DIR)table-private.h $(INC_DIR)list.h
	$(CC) $(CFLAGS) $(SRC_DIR)table-private.c -o $(OBJ_DIR)table-private.o

table.o: $(INC_DIR)table.h $(INC_DIR)table-private.h $(INC_DIR)list.h
	$(CC) $(CFLAGS) $(SRC_DIR)table.c -o $(OBJ_DIR)table.o

serialization.o: $(INC_DIR)serialization.h
	$(CC) $(CFLAGS) $(SRC_DIR)serialization.c -o $(OBJ_DIR)serialization.o

#####################################

test_data:
	$(CC) $(LFLAGS) $(DEPS_test_data) $(SRC_DIR)test_data.c -o $(BIN_DIR)test_data

test_entry:
	$(CC) $(LFLAGS) $(DEPS_test_entry) $(SRC_DIR)test_entry.c -o $(BIN_DIR)test_entry

test_list:
	$(CC) $(LFLAGS) $(DEPS_test_list) $(SRC_DIR)test_list.c -o $(BIN_DIR)test_list

test_table:
	$(CC) $(LFLAGS) $(DEPS_test_table) $(SRC_DIR)test_table.c -o $(BIN_DIR)test_table

test_serialization:
	$(CC) $(LFLAGS) $(DEPS_test_serialization) $(SRC_DIR)test_serialization.c -o $(BIN_DIR)test_serialization

clean:
	@rm -f $(OBJ_DIR)*.o $(BIN_DIR)*