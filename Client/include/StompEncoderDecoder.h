//
// Created by yael on 08/01/2020.
//

#ifndef BOOST_STOMPENCODERDECODER_H
#define BOOST_STOMPENCODERDECODER_H

#include <unordered_map>
#include <vector>
#include "MessageFrame.h"

using namespace std;

class StompEncoderDecoder {
private:
    int len = 0;
    vector<char> charArray = vector<char>();

public:
    //decode message from server
    MessageFrame* decode( string &frame);

    //encode message from user
    vector<char> encode(MessageFrame &message);

    //destructor
    virtual ~StompEncoderDecoder();
};


#endif //BOOST_STOMPENCODERDECODER_H
