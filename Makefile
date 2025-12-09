CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g
LDFLAGS = -lm

# Source files
LIB_SOURCES = int512.c
TEST_SOURCES = test_int512.c test_main.c

# Object files
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Target executable
TEST_EXECUTABLE = .bin/test_int512

.PHONY: all clean test

all: $(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(LIB_OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c int512.h ctest.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

clean:
	rm -f $(LIB_OBJECTS) $(TEST_OBJECTS) $(TEST_EXECUTABLE)
