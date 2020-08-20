//
// Created by yael on 11/01/2020.
//

#include <boost/accumulators/numeric/functional/vector.hpp>
#include "Book.h"

//empty constructor
Book::Book() : name(""), topic(""){
}

//constructor
Book::Book(string name, string topic) : name(name), topic(topic) {
}

//Book &Book::operator=(Book &other) {
//    if (this != &other){
//
//        name = other.getName();
//        topic = other.getTopic();
//        //hasTheBook = other.getHasTheBook();
//    }
//    return *this;
//}

/**
 * getter for the book name
 * @return string book name
 */
string Book::getName() const {
    return name;
}

/**
 * setter for the book name
 * @param name a name to give the book
 */
void Book::setName(const string &name) {
    Book::name = name;
}

/**
 * getter for the book genre
 * @return string book topic
 */
string Book::getTopic() const {
    return topic;
}

/**
 * setter for the book genre
 * @param topic the genre of the book
 */
void Book::setTopic(const string &topic) {
    Book::topic = topic;
}

//Book::~Book() {
//    name= nullptr;
//    topic= nullptr;
//}

