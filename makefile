CC = g++
CFLAGS = -std=c++11

all: server.cpp client.cpp
	$(CC) $(CFLAGS) server.cpp -o server
	$(CC) $(CFLAGS) client.cpp -o client

clean:
	rm -f server client