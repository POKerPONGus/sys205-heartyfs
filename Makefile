CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -Wshadow
BIN_DIR := bin
SRC_DIR := src
OP_DIR := $(SRC_DIR)/op

UTILS := $(shell find src/ -maxdepth 1 -type f -not -name "*init*")
UTIL_OBJS := $(patsubst %.c, %.o, $(UTILS))

BASES := $(shell find src/ -maxdepth 1 -type f -name "*init*" -printf "%f\n")
BASE_BINS := $(patsubst %.c, %, $(BASES))

OPS := $(shell find src/op -type f -printf "%f\n")
OP_BINS := $(patsubst %.c, %, $(OPS))

.PHONY: all
all: $(BASE_BINS) $(OP_BINS)

.PHONY: run
run: test.out
	./test.out

test.out: %.out : %.c $(UTIL_OBJS)
	$(CC) $(CFLAGS) $(^F) -o $@ -I include/

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $(@F) -I include/

$(BASE_BINS): % : $(SRC_DIR)/%.o $(UTIL_OBJS)
	$(CC) $(CFLAGS) $(^F) -o $(BIN_DIR)/$@ -I include/

$(OP_BINS): % : $(OP_DIR)/%.o $(UTIL_OBJS)
	$(CC) $(CFLAGS) $(^F) -o $(BIN_DIR)/$@ -I include/