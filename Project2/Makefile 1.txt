CFLAGS = -w
CC = gcc

all: client server

client: client.c struct.h
	${CC} ${CFLAGS} client.c -o client -lncurses

server: server.c struct.h
	${CC} ${CFLAGS} server.c -o server

clean:
	rm -f *.o *~ server client