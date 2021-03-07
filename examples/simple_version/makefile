all:server client 

server:server.cpp
	g++ -std=c++17 server.cpp -o server 

client:client.cpp 
	g++ -std=c++17 client.cpp -lpthread -o client

clean:
	rm server client 