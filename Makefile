CC = gcc          # The compiler
CFLAGS = -Wall    # Compiler flags: -Wall shows all warnings
TARGET = pingpong # The output binary name

# The first rule is the default target
all: $(TARGET)

# Rule for creating the binary
$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

# Rule to clean up the generated files
clean:
	rm -f $(TARGET)