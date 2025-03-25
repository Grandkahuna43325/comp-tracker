CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -g
LDFLAGS = 
SRC_DIR = src
BUILD_DIR = build
BIN = $(BUILD_DIR)/main

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

all: $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN): $(BUILD_DIR) $(OBJS)
	$(CC) $(OBJS) -o $(BIN) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: $(BIN)
	$(BIN)

.PHONY: all clean run
