//
// Created by yael on 08/01/2020.
//

#include <iostream>
#include "MessageFrame.h"

MessageFrame::MessageFrame(const string &command, const vector<pair<string, string>> &headers, const string &body) : command (command), headers(headers), body(body){
}

MessageFrame::MessageFrame(const MessageFrame &other) :command(other.getCommand()), headers(vector<pair<string,string>>()),body(other.getBody()){
    for (const auto & i : other.getHeaders()) {
        headers.emplace_back(i.first,i.second);
    }
}

const string &MessageFrame::getCommand() const {
    return command;
}

void MessageFrame::setCommand(const string &command) {
    MessageFrame::command = command;
}

const vector<pair<string, string>> &MessageFrame::getHeaders() const {
    return headers;
}

void MessageFrame::setHeaders(const vector<pair<string, string>> &headers) {
    MessageFrame::headers = headers;
}

const string &MessageFrame::getBody() const {
    return body;
}

void MessageFrame::setBody(const string &body) {
    MessageFrame::body = body;
}

string MessageFrame::getHeader(const string &key) {
    for (const auto &i : headers) {
        if (i.first == key) {
            return i.second;
        }
    }
    return "";
}



MessageFrame::~MessageFrame() {
    headers.clear();
}

string MessageFrame::toString() {
    string ret="";
    ret+= command+'\n';
    for (auto & header : headers) {
        ret += header.first + ": "+header.second+'\n';
    }
    ret+=body+'\n';
    return ret;
}







