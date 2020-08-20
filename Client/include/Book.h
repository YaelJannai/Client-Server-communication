//
// Created by yael on 11/01/2020.
//

#ifndef BOOST_BOOK_H
#define BOOST_BOOK_H

#include <string>

using namespace std;

class Book {
private:
    string name;
    string topic;

public:

    //constructors
    Book();

    Book(string name, string topic);

    //returns book name
    string getName() const;

    //sets book name
    void setName(const string &name);

    //return the book's genre
    string getTopic() const;

    //sets book genre
    void setTopic(const string &topic);

//    virtual ~Book();
};


#endif //BOOST_BOOK_H
