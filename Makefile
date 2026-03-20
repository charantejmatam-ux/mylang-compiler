# Simple Compiler Makefile
CC = gcc
CFLAGS = -Wall -Wextra -g -I./src
TARGET = scc  # Simple C Compiler

# Source files
SRCS = src/main.c \
       src/lexer/lexer.c \
       src/parser/parser.c \
       src/ast/ast.c \
       src/symbol_table/symbol_table.c \
       src/semantic/semantic.c \
       src/codegen/codegen.c \
       src/utils/utils.c

OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link object files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET) output/*.asm output/*.o output/*.out

# Run tests
test: $(TARGET)
	@echo "Running tests..."
	@./$(TARGET) tests/test1.c
	@./$(TARGET) tests/test2.c
	@./$(TARGET) tests/test3.c

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: all clean test install
