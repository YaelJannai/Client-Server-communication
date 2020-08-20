//
// Created by yael on 08/01/2020.
//

#include <iostream>
#include "StompEncoderDecoder.h"

using namespace std;

/**
 * decodes the message the user enterd in order to send it
 * @param frame the message the user typed
 * @return the message decoded, can be sent to server
 */
MessageFrame* StompEncoderDecoder::decode(string &frame) {

    //cuts the messages according to the parts of it
    string command = frame.substr(0, frame.find('\n'));
    frame = frame.substr(frame.find('\n') + 1);
    vector<pair<string, string>> headers = vector<pair<string, string>>();
    string line = frame.substr(0, frame.find('\n'));

   //until there is nothing more to parse, continue to parse the given message
    while (line != "") {
        string key = line.substr(0, line.find(':'));
        string value = line.substr(line.find(':') + 1);
        headers.emplace_back(key, value);
        frame = frame.substr(frame.find('\n') + 1);
        line = frame.substr(0, frame.find('\n'));
    }
    string body = frame.substr(1, frame.find_last_of('\n')-1);

    //checks if there are to many \n in the message decoded
    //DO NOT FORGET TO FIX
    if(body.size()>0){
        while (body.at(body.size()-1) == '\n')
        {
            body = body.substr(0, body.size()-1);
        }
    }

    //message decoded and ready for user
    auto *messageFrame = new MessageFrame(command, headers, body);
    return messageFrame;
}

/**
 * encodes a message the process from client created
 * in order to send server
 * @param message the message to encode
 * @return vector of chars - the encoded message
 */
vector<char> StompEncoderDecoder::encode(MessageFrame &message) {
    vector<char> charMessage = vector<char>();
    int pointer = 0;

    //gets the message body to encode
    string msgCommand(message.getCommand());
    for (char c : msgCommand) {
        charMessage.push_back(c);
        pointer++;
    }
    charMessage.push_back('\n');
    pointer++;

    vector<pair<string, string>> msgHeaders(message.getHeaders());
    //runs on the headers and content to encode
    for (pair<string, string> element : msgHeaders) {
        string msgKey(element.first);
        string msgValue(element.second);
        for (char c : msgKey) {
            charMessage.push_back(c);
            pointer++;
        }
        charMessage.push_back(':');
        pointer++;
        for (char c : msgValue) {
            charMessage.push_back(c);
            pointer++;
        }
        charMessage.push_back('\n');
        pointer++;
    }
    charMessage.push_back('\n');
    pointer++;

    //runs on the body to encode
    string msgBody(message.getBody());
    for (char c : msgBody) {
        charMessage.push_back(c);
        pointer++;
    }
    charMessage.push_back('\n');
    pointer++;

    //insert delimeter to message encoded
    charMessage.push_back('\0');
    delete(&message);
    return charMessage;
}

//destructor
StompEncoderDecoder::~StompEncoderDecoder() {
    charArray.clear();
}



