CC = mpicc
CFLAGS = -Wall -O2 -I./include
LDFLAGS = -lm

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = .

# Source files
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/wav.c $(SRC_DIR)/window.c $(SRC_DIR)/fft.c \
          $(SRC_DIR)/bpm.c $(SRC_DIR)/stft.c $(SRC_DIR)/mpi_utils.c

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Executable
TARGET = $(BIN_DIR)/main

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Link
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Phony targets
.PHONY: all clean
