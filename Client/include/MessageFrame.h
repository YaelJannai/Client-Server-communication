//
// Created by yael on 08/01/2020.
//

#ifndef BOOST_MESSAGEFRAME_H
#define BOOST_MESSAGEFRAME_H

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class MessageFrame {
private:

    string command;
    vector<pair<string, string>> headers;
    string body;

public:

    MessageFrame(const string &command, const vector<pair<string, string>> &headers, const string &body);

    MessageFrame(const MessageFrame &other);

    virtual ~MessageFrame();

    const string &getCommand() const;

    void setCommand(const string &command);

    const vector<pair<string, string>> &getHeaders() const;

    void setHeaders(const vector<pair<string, string>> &headers);

    const string &getBody() const;

    void setBody(const string &body);

    string getHeader(const string &key);

    string toString();
};


#endif //BOOST_MESSAGEFRAME_H
