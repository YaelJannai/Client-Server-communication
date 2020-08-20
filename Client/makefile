CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system

all: StompBookClubClient
	g++ -pthread -o bin/StompBookClubClient bin/MessageFrame.o bin/StompEncoderDecoder.o bin/ConnectionHandler.o bin/Book.o bin/User.o bin/StompProtocol.o bin/TaskSocket.o bin/StompClient.o $(LDFLAGS) 


StompBookClubClient: bin/MessageFrame.o bin/StompEncoderDecoder.o bin/ConnectionHandler.o bin/Book.o bin/User.o bin/StompProtocol.o bin/TaskSocket.o bin/StompClient.o

bin/MessageFrame.o: src/MessageFrame.cpp
	g++ -pthread $(CFLAGS) -o bin/MessageFrame.o src/MessageFrame.cpp

bin/StompEncoderDecoder.o: src/StompEncoderDecoder.cpp
	g++ -pthread $(CFLAGS) -o bin/StompEncoderDecoder.o src/StompEncoderDecoder.cpp

bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ -pthread $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

bin/Book.o: src/Book.cpp
	g++ -pthread $(CFLAGS) -o bin/Book.o src/Book.cpp

bin/User.o: src/User.cpp
	g++ -pthread $(CFLAGS) -o bin/User.o src/User.cpp

bin/StompProtocol.o: src/StompProtocol.cpp
	g++ -pthread $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

bin/TaskSocket.o: src/TaskSocket.cpp
	g++ -pthread $(CFLAGS) -o bin/TaskSocket.o src/TaskSocket.cpp

bin/StompClient.o: src/StompClient.cpp
	g++ -pthread $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

.PHONY: clean
clean:
	rm -f bin/*
