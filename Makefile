CC = gcc
CFLAGS = -Wall -g -O0
CFILES = main.c
OBJFILES = $(CFILES:.c=.o)  # Converts all .c files to .o files
TARGET = pingpong

all: $(TARGET)

# Rule to build the target binary from object files
$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES)

# Rule to build object files from source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove object files and the binary
clean:
	rm -f $(TARGET) $(OBJFILES)