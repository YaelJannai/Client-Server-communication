//
// Created by yael on 08/01/2020.
//

#ifndef BOOST_STOMPPROTOCOL_H
#define BOOST_STOMPPROTOCOL_H


#include "MessageFrame.h"
#include "User.h"
#include "ConnectionHandler.h"

class StompProtocol {

private:
    bool isDone;
    vector<string> commandFromServer  = {"", "CONNECTED", "MESSAGE", "RECEIPT", "ERROR"};
    vector<string> commandFromClient = {"", "login", "join", "exit", "add", "borrow", "return", "status", "logout"};
    User user = User();
    ConnectionHandler &connectionHandler;
    int subscribeIdCounter = 0;
    int receiptIdCounter = 0;

    void connected();
    MessageFrame* message(MessageFrame &msg);
    void receipt(MessageFrame &msg);
    void error(MessageFrame &msg);
    void disconnect();

    void login(string &msg);
    void join(string &msg);
    void exit(string &msg);
    void unsubscribe(int id);
    void add(string &msg);
    void borrow(string &msg);
    void returnBook(string &msg);
    void status(string &msg);
    void logout(string &msg);

public:
    //constructor
    StompProtocol(User &user, ConnectionHandler &connectionHandle);

    //if the protocol should keep running
    bool shouldTerminate();
    //process - handles the actual content of the messages
    MessageFrame* processFromServer(MessageFrame& msg);
    void processFromClient(string& msg);

    //destructor
    virtual ~StompProtocol();
};


#endif //BOOST_STOMPPROTOCOL_H
