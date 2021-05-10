all: client server
		

client:
	g++ client.cpp -o client

server:
	g++ server.cpp -o server

.PHONY: all client server