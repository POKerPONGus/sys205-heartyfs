.SECOND_EXPANSION:

CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -Wshadow -Wunused-function -fanalyzer -fsanitize=address -g
BIN_DIR := bin
OBJ_DIR := obj
SRC_DIR := src
OP_DIR := $(SRC_DIR)/op
UTIL_DIR := $(SRC_DIR)/util

UTILS := $(shell find $(UTIL_DIR) -type f)
UTIL_OBJS := $(patsubst $(UTIL_DIR)/%.c, $(OBJ_DIR)/%.o, $(UTILS))

OPS := $(shell find $(OP_DIR) -type f)
OP_OBJS := $(patsubst $(OP_DIR)/%.c, $(OBJ_DIR)/%.o, $(OPS))

BASE := $(SRC_DIR)/heartyfs.c
BASE_OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(BASE))
BIN := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%, $(BASE))

.PHONY: all
all: $(BIN)

.PHONY: clean
clean:
	rm $(BIN_DIR)/* $(OBJ_DIR)/*
	
# Programs Binaries

$(BIN): $(BIN_DIR)/% : $(BASE_OBJ) $(OP_OBJS) $(UTIL_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -I include/

# Object files

$(OP_OBJS): $(OBJ_DIR)/%.o : $(OP_DIR)/%.c
	$(CC) $(CFLAGS) $< -c -o $@ -I include/

$(UTIL_OBJS): $(OBJ_DIR)/%.o : $(UTIL_DIR)/%.c
	$(CC) $(CFLAGS) $< -c -o $@ -I include/

$(BASE_OBJ): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -c -o $@ -I include/


