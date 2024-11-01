.SECOND_EXPANSION:

CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -Wshadow
BIN_DIR := bin
OBJ_DIR := obj
SRC_DIR := src
OP_DIR := $(SRC_DIR)/op


DISK = /tmp/heartyfs
DISK_CREATOR = $(patsubst %.c, $(BIN_DIR)/%, heartyfs_create_disk.c)

UTILS := $(shell find $(SRC_DIR) -maxdepth 1 -type f -name "*utils*")
UTIL_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(UTILS))

BASES := $(shell find $(SRC_DIR) -maxdepth 1 -type f -name "*init*")
BASE_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(BASES))
BASE_BINS := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%, $(BASES))

OPS := $(shell find $(OP_DIR) -type f)
OP_OBJS := $(patsubst $(OP_DIR)/%.c, $(OBJ_DIR)/%.o, $(OPS))
OP_BINS := $(patsubst $(OP_DIR)/%.c, $(BIN_DIR)/%, $(OPS))

.PHONY: all
all: $(BASE_BINS) $(OP_BINS) $(DISK)

.PHONY: clean
clean:
	rm $(BIN_DIR)/* $(OBJ_DIR)/*

.PHONY: run
run: test.out $(DISK)
	./test.out

test.out: %.out : %.c $(UTIL_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -I include/
		
$(DISK): $(DISK_CREATOR)
	./$<

# Programs Binaries

$(BASE_BINS) $(OP_BINS): $(BIN_DIR)/% : $(OBJ_DIR)/%.o $(UTIL_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -I include/

$(DISK_CREATOR): $(BIN_DIR)/% : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@ -I include/

# Object files

$(OP_OBJS): $(OBJ_DIR)/%.o : $(OP_DIR)/%.c
	$(CC) $(CFLAGS) $< -c -o $@ -I include/

$(BASE_OBJS) $(UTIL_OBJS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -c -o $@ -I include/


