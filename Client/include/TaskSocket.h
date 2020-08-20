//
// Created by yaeljan@wincs.cs.bgu.ac.il on 12/01/2020.
//

#ifndef BOOST_TASKSOCKET_H
#define BOOST_TASKSOCKET_H


#include "StompEncoderDecoder.h"
#include "StompProtocol.h"
#include "ConnectionHandler.h"

class TaskSocket {

private:
    ConnectionHandler *socketHandler;
    StompEncoderDecoder *encdec;
    StompProtocol *protocol;

public:
    //constructor
    TaskSocket(ConnectionHandler *socketHandler, StompEncoderDecoder *encdec,  StompProtocol *protocol);

    //a run method - the action of the thread
    void run();
};


#endif //BOOST_TASKSOCKET_H
