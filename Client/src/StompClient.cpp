//
// Created by yael on 08/01/2020.
//

#include <stdlib.h>
#include <ConnectionHandler.h>
#include <StompEncoderDecoder.h>
#include <StompProtocol.h>
#include <thread>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <TaskSocket.h>

using namespace std;

int main (int argc, char *argv[]) {
//    if (argc < 3) {
//        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
//        return -1;
//    }
//    std::string host = argv[1];
//    short port = atoi(argv[2]);

    //assign variables in order to read from user - keyboard
    const short bufsize = 1024;
    char buf[bufsize];
    cin.getline(buf, bufsize);
    string line(buf);
    bool endingProgram = false;

    //will continue running until receives 'bye' to end the loop
    while (!endingProgram) {
        //a chance to write another command if tried to do something before login
        while (line.substr(0, 5) != "login") {
            //if the user writes 'bye' closes the program
            if (line.substr(0, 3) == "bye") {
                endingProgram = true;
                return 0;
            }
            cout << "user is not logged in yet\n";
            cin.getline(buf, bufsize);
            line = buf;
        }

        //gets from user's input the host to connect to
        string host = line.substr(line.find(' ') + 1, line.find(':') - 6);
        short port = stoi(line.substr(line.find(':') + 1, line.find(' ')));

        //creates StompEncoderDecoder for keyboard and connection handler that receives it's needed parameters to run
        auto *encdecKeyboard = new StompEncoderDecoder();
        auto *connectionHandler = new ConnectionHandler(host, port,*encdecKeyboard);

        //tries to connect to the server
        if (!connectionHandler->connect()) {
            std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
            return 1;
        }

        //creates StompEncoderDecoder for socket
        auto *encdecSocket = new StompEncoderDecoder();

        //creates a default user to send the protocol, and a new protocol - contains the main process
        User *user = new User();
        auto *aProtocol = new StompProtocol(*user, *connectionHandler);

        //a new task for the socket to run in, and a thread to run it
        TaskSocket taskS = TaskSocket(connectionHandler, encdecSocket, aProtocol);
        thread tSock =  thread (&TaskSocket::run, &taskS);

        //call the process that handles reading from client, with the unput
        aProtocol->processFromClient(line);
        cin.getline(buf, bufsize);
        line = buf;

        //while can, runs the loop and read more data from user keyboard
        while (!aProtocol->shouldTerminate()&&!user->isAboutToLogout()) {
            aProtocol->processFromClient(line);
            cin.getline(buf, bufsize);
            line = buf;
        }

        //closes and deletes everything we don't need at the end
        tSock.join();
        delete(user);
        user = nullptr;
        delete(aProtocol);
        aProtocol = nullptr;
        delete(connectionHandler);
        connectionHandler= nullptr;
        delete(encdecKeyboard);
        encdecKeyboard= nullptr;
        delete(encdecSocket);
        encdecSocket= nullptr;

    }
    return 0;
}
