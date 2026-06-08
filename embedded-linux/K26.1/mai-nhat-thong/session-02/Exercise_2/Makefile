CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude

BUILD_DIR = build
BIN_DIR = bin

SRC = src/stringutils.c
OBJ = $(BUILD_DIR)/stringutils.o
SHARED_LIB = $(BIN_DIR)/libstringutils.so
TARGET = $(BIN_DIR)/main

# Sample string used for 'make run'
SAMPLE_STR = "Embedded_Linux_2026"

.PHONY: all clean run

all: $(TARGET)

# Compile with -fPIC for shared library
$(OBJ): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# Create shared library (.so)
$(SHARED_LIB): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) -shared -o $@ $<

# Link main with -rpath pointing to ./bin so it finds the .so at runtime
$(TARGET): main.c $(SHARED_LIB)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $< -L$(BIN_DIR) -lstringutils -Wl,-rpath,'$$ORIGIN' -o $@

run: $(TARGET)
	@echo "Running binary with argument: $(SAMPLE_STR)"
	./$(TARGET) $(SAMPLE_STR)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)