EXE = htproxy

CC = gcc
CFLAGS = -O3 -Wall -g

# directories
SRC_DIR = src

# src and obj files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(SRC_DIR)/%.o)

# link to executable
$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)

# compile to object files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# clean up directory
clean:
	rm -f $(EXE) $(OBJ)

# Format code using clang-format
format:
	clang-format -style=file -i $(SRC_DIR)/*.c $(SRC_DIR)/*.h