CC = gcc
CFLAGS =-g 
#-Wall 

SERVER_OBJS= hashmap_utils.o server.o
CLIENT_OBJS= hashmap_utils.o client.o

all: client server

client: $(CLIENT_OBJS)
	$(CC) -o $@ $^

server: $(SERVER_OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f client server $(SERVER_OBJS) $(CLIENT_OBJS)

