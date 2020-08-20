//
// Created by yaeljan@wincs.cs.bgu.ac.il on 08/01/2020.
//

#ifndef BOOST_USER_H
#define BOOST_USER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include "MessageFrame.h"
#include "Book.h"
#include <mutex>

using namespace std;

class User {
private:

    bool isConnected;
    string userName;
    string passcode;
    unordered_map<string, unordered_map<string, Book*>> inventory;
    unordered_map<int, string> subscribedTo;
    unordered_map<int, MessageFrame> receiptMap;
    unordered_map<string, string> borrowedBooks;
    vector<string> wantedBooks;
    bool aboutToLogout;

    static mutex mutexInventory;
    static mutex mutexReceipt;
    static mutex mutexBorrowed;
    static mutex mutexWanted;

public:

    //constructor
    User();
    User(string userName, string passcode);
    //copy assignment operator
    User& operator = (User &other);

    //destructor
    virtual ~User();

    //get-set for the user state - connected or not
    const bool getIsConnected();
    void setIsConnected(const bool &ans);

    //get-set for the user name
    const string &getUserName();
    void setUserName(const string &userName);

    //get-set for the user password
    void setPasscode(const string &passcode);
    const string &getPasscode();

    //to string for the status command
    string toStringStatus(const string& genre);

    //get-add-remove of the user's books inventory
    const unordered_map<string, unordered_map<string, Book*>> &getInventory();
    //void addInventory(const Book& book);
    void addInventory(Book& book);
    void removeInventory(const string& genre, const string& bookName);

    //get-add-remove of the user's topics he is subscribed to
    const unordered_map<int, string>&getSubscribedTo() ;
    void addSubscribedTo(int key, const string &value);
    void removeSubscribeTo(int key);

    //get-add-remove of the receipt the user asked for
    const unordered_map<int, MessageFrame> &getReceiptMap();
    void addReceiptMap(int key ,const MessageFrame&value);
    void removeReceiptMap(int key);

    //get-add-remove of the books the user borrowed
    const unordered_map<string, string> &getBorrowedBooks();
    const string &getBorrowedBookOwner(const string &bookName);
    void addBorrowedBooks(const Book &book, const string &owner);
    void removeBorrowedBooks(const string &book);

    //get-add-remove of the books the user want to borrow
    const vector<string> &getWantedBooks();
    void addWantedBooks(const string &wantedBook);
    void removeWantedBooks(const string &wantedBook);

    //if the user is about to disconnect system
    bool isAboutToLogout() const;
    void setAboutToLogout(bool aboutToLogout);

};


#endif //BOOST_USER_H
