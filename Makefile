CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic
LDLIBS = -pthread -lm

TARGET = parallel_matrix_vector
SOURCE = src/parallel_matrix_vector.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET) $(LDLIBS)

run: $(TARGET)
	./$(TARGET) 1000 4

test: $(TARGET)
	./$(TARGET) 128 1
	./$(TARGET) 128 2
	./$(TARGET) 128 4

clean:
	rm -f $(TARGET)

.PHONY: all run test clean
