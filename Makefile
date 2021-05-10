all: client server
		

client: createfolder
	g++ src/client.cpp -o build/client

server: createfolder
	g++ src/server.cpp -o build/server

createfolder:
	mkdir -p build 

clean:
	rm -R build

.PHONY: all client server clean