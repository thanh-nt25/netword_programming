# Define variables
CC = gcc
CFLAGS = -g
TARGET = output/basic_client
SRC = basic_client.c

# Build the target
all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p output  # Create output directory if it doesn't exist
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lpthread

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf output

