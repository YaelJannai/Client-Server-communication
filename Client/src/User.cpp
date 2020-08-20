//
// Created by yaeljan@wincs.cs.bgu.ac.il on 08/01/2020.
//

#include <User.h>
#include <mutex>
#include <utility>

mutex User::mutexInventory;
mutex User::mutexReceipt;
mutex User::mutexBorrowed;
mutex User::mutexWanted;

//empty constructor
User::User(): isConnected(false), userName(""), passcode("") , inventory(unordered_map<string,unordered_map<string,Book*>>()), subscribedTo(unordered_map<int, string>()), receiptMap(unordered_map<int, MessageFrame>()), borrowedBooks(unordered_map<string,string>()), wantedBooks(vector<string>()),aboutToLogout(false){
}

//constructor
User::User(string userName, string passcode) : isConnected(false), userName(std::move(userName)), passcode(std::move(passcode)), inventory(unordered_map<string,unordered_map<string,Book*>>()) ,subscribedTo(unordered_map<int, string>()),receiptMap(unordered_map<int, MessageFrame>()), borrowedBooks(unordered_map<string,string>()),  wantedBooks(vector<string>()), aboutToLogout(false){
}

//copy assugnment operator
User &User::operator=(User &other) {
    if (this != &other){
        inventory.clear();
        subscribedTo.clear();

        isConnected = other.getIsConnected();
        userName = other.getUserName();
        passcode = other.getPasscode();
        inventory = other.getInventory();
        subscribedTo = other.getSubscribedTo();
    }
    return *this;
}

/**
 * getter for the user's state - connected or not
 * @return the user's state - connected or not
 */
const bool User::getIsConnected(){
    return isConnected;
}

/**
 * setter for the user's state - connected or not
 * @param ans true or false
 */
void User::setIsConnected(const bool &ans) {
    User::isConnected = ans;
}

/**
 * getter for the user's name
 * @return the user name
 */
const string &User::getUserName() {
    return userName;
}

/**
 * setter for the user's name
 * @param userName a name to give the user
 */
void User::setUserName(const string &userName) {
    User::userName = userName;
}

/**
 * getter for the user's password
 * @return the password
 */
const string &User::getPasscode() {
    return passcode;
}

/**
 * setter for the user's password
 * @param passcode a password to give the user
 */
void User::setPasscode(const string &passcode) {
    User::passcode = passcode;
}

/**
 * to string for the status command
 * @param genre the genre to print books from
 * @return a string with all user's books in that genre
 */
string User::toStringStatus(const string& genre) {
    string toReturn(getUserName()+": ");
    if(inventory.find(genre)!=inventory.end())
    {
        for (auto& it: inventory.at(genre)) {
            toReturn += it.first+ ',';
        }
    }
    toReturn = toReturn.substr(0, toReturn.size()-1);
    return toReturn;
}

/**
 * getter for the user's books inventory
 * @return user's books inventory
 */
const unordered_map<string, unordered_map<string, Book*>> &User::getInventory() {
    return inventory;
}

/**
 * adds a new book to the user inventory
 * @param book the book to insert the user's inventory
 */
void User::addInventory(Book &book) {
    string n = book.getName();
    //pair<string, Book> p(book.getName(), book);
    pair<string, Book*> p(book.getName(), &book);
    //locks the inventory so no changes will occur while insertion
    lock_guard<mutex> lockGuard(mutexInventory);
    //if this topic doesn't exist yet, create a new one and insert the book to it.
    if (inventory.find(book.getTopic()) == inventory.end()){
        unordered_map<string, Book*> booksMap = unordered_map<string, Book*>();
        //unordered_map<string, Book> booksMap = unordered_map<string, Book>();
        booksMap.insert(p);
        //pair<string, unordered_map<string, Book>> topicPair(book.getTopic(), booksMap);
        pair<string, unordered_map<string, Book*>> topicPair(book.getTopic(), booksMap);
        inventory.insert(topicPair);
    }
    //the topic already exist, insert the book if doesn't exist
    else {
        if(inventory[book.getTopic()].find(book.getName()) == inventory[book.getTopic()].end()) {
            inventory[book.getTopic()].insert(p);
        }
    }
}
/**
 * removes achosen book from the user books inventory
 * @param genrethe genre the book belongs to
 * @param bookName the name of the book to delete
 */
void User::removeInventory(const string& genre, const string& bookName) {
    lock_guard<mutex> lockGuard(mutexInventory);
    if (inventory.find(genre) != inventory.end()) {
        inventory[genre].erase(bookName);
    }
}

/**
 * getter for all the topics the user susbcribed to
 * @return the subscribed map
 */
const unordered_map<int, string> &User::getSubscribedTo(){
    return subscribedTo;
}

/**
 * add a new topic the user subscribed to
 * @param key the user's id for that topic
 * @param value the topic name
 */
void User::addSubscribedTo(int key, const string &value){
    pair<int , string>p(key, value);
   if(subscribedTo.find(key)==subscribedTo.end())
   {
       subscribedTo.insert(p);
   }
}

/**
 * after unsubscribing, erase the tipc name from the vector
 * @param key the user's id in that topic
 */
void User::removeSubscribeTo(int key) {
    subscribedTo.erase(key);
}

/**
 * getter for all the user's receipts
 * @return the receipts map
 */
const unordered_map<int, MessageFrame> &User::getReceiptMap() {
    return receiptMap;
}

/**
 * add a mew receipt the user waits for
 * @param key the number of the receipt
 * @param value the message he want a receipt for
 */
void User::addReceiptMap(int key, const MessageFrame &value) {
    pair<int , MessageFrame>p(key, value);
    lock_guard<mutex> lockGuard(mutexReceipt);
    receiptMap.insert(p);
}

/**
 * remove a message we received receipt for
 * @param key the number of the receipt
 */
void User::removeReceiptMap(int key) {
    lock_guard<mutex> lockGuard(mutexReceipt);
    receiptMap.erase(key);
}

/**
 * getter for all the books the user want to borroe
 * @return the vector of wanted books
 */
const vector<string> &User::getWantedBooks(){
    return wantedBooks;
}

/**
 * add a book the user want in his inventory
 * @param wantedBook  the book name
 */
void User::addWantedBooks(const string &wantedBook) {
    lock_guard<mutex> lockGuard(mutexWanted);
    wantedBooks.push_back(wantedBook);
}

/**
 * remove a book from the vector, means the user succeded to borrow it
 * @param wantedBook the book name
 */
void User::removeWantedBooks(const string &wantedBook) {
    lock_guard<mutex> lockGuard(mutexWanted);
    for (size_t i = 0; i < wantedBooks.size(); ++i) {
        if (wantedBooks.at(i) == wantedBook) {
            wantedBooks.erase(wantedBooks.begin() + i);
        }
    }
}

/**
 * getter for all the books the user borrowed
 * @return unordered_map of book names and owners names
 */
const unordered_map<string,string> &User::getBorrowedBooks(){
    return borrowedBooks;
}

/**
 * add a book the user want to borrow to he's map
 * @param book the book to borrow
 * @param owner the owner's name
 */
void User::addBorrowedBooks(const Book &book, const string &owner) {
     pair<string , string>p(book.getName(), owner);
     //lock so no changes will occur while inserting the map
     lock_guard<mutex> lockGuard(mutexBorrowed);
     borrowedBooks.insert(p);
}

/**
 * remove a book the user borrowed, return it to the owners
 * @param book the name of the book we borrowed
 */
void User::removeBorrowedBooks(const string &book) {
    lock_guard<mutex> lockGuard(mutexBorrowed);
    //changed for loop
    for (auto& it: borrowedBooks) {
        if(it.first == book){
            borrowedBooks.erase(book);
        }
    }
}

/**
 * checks how is the owner of a book the user borrowed
 * @param bookName the book to check owners
 * @return the owner
 */
const string &User::getBorrowedBookOwner(const string &bookName)  {
    lock_guard<mutex> lockGuard(mutexBorrowed);
    return borrowedBooks.at(bookName);

}

//destructor
User::~User() {
    for(auto&iter: inventory) {
        iter.second.clear();
    }
    inventory.clear();
    subscribedTo.clear();
    receiptMap.clear();
    borrowedBooks.clear();
    wantedBooks.clear();
}

/**
 * getter to tell if the user is about to logout or not
 * @return the user state
 */
bool User::isAboutToLogout() const {
    return aboutToLogout;
}

/**
 * setter to change if the user is about to logout or not
 * @param aboutToLogout true or false
 */
void User::setAboutToLogout(bool aboutToLogout) {
    User::aboutToLogout = aboutToLogout;
}
