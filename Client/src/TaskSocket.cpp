//
// Created by yaeljan@wincs.cs.bgu.ac.il on 12/01/2020.
//

#include "TaskSocket.h"

#include <utility>

//constructor
TaskSocket::TaskSocket(ConnectionHandler *socketHandler, StompEncoderDecoder *encdec,  StompProtocol *protocol): socketHandler(socketHandler),encdec(encdec),protocol(protocol){
}

/**
 * run method - the action of the thread.
 * Here all the logic and work should be done by the socket thread
 */
void TaskSocket::run() {

    //will stay in the loop until receives a signal to stop
    while (!protocol->shouldTerminate()) {
        string frame;
        //tries to read from the server
        if(!socketHandler->getFrameAscii(frame,'\0'))
        {
            cout<<"error: not connected to server\n";
        }
        //decodes the message so the user will understand
        MessageFrame *msgFromServer = encdec->decode(frame);

        //calls the process, to handle the server's response
        MessageFrame *answerToServer = protocol->processFromServer(*msgFromServer);
        delete(msgFromServer);

        //if the process didn't return null it means the socket should send something to the server
        if (answerToServer != nullptr) {
            //encode the message before sending
            auto *bytesToServer = new vector<char>(encdec->encode(*answerToServer));
            const short vecSize = bytesToServer->size();
            char bytes2[vecSize];

            for (int i = 0; i < vecSize; ++i) {
                bytes2[i] = bytesToServer->at(i);
            }
            bytesToServer->clear();
            delete (bytesToServer);
            bytesToServer = nullptr;

            //sends the server the user's response
            socketHandler->sendBytes(bytes2, vecSize);
        }
    }
}

