//
// Created by yael on 08/01/2020.
//

#include <vector>
#include <iostream>
#include "StompProtocol.h"

using namespace std;

//constructor
StompProtocol::StompProtocol(User &user, ConnectionHandler &connectionHandle) : isDone(false), user(user), connectionHandler(connectionHandle){
}

/**
 * returns the protocol status
 * @return isDone
 */
bool StompProtocol::shouldTerminate() {
    return isDone;
}

/**
 * receives the message sent from the server, handles it
 * @param msg the message from server
 * @return a message frame to send back to server
 */
MessageFrame *StompProtocol::processFromServer(MessageFrame &msg) {
    for (size_t i = 1; i < commandFromServer.size(); ++i) {
        //uses switch and case by the content
        if (commandFromServer.at(i) == msg.getCommand()){
            switch (i)
            {
                case 1: //CONNECTED
                    connected();
                    break;
                case 2: //MESSAGE
                    return message(msg);
                    break;
                case 3: //RECEIPT
                    receipt(msg);
                    break;
                case 4: //ERROR
                //why to disconnect?
                    error(msg);
                    //disconnect();
                    break;
            }
        }
    }
    return nullptr;
}

/**
 * receives the message from user (what the user typed), handles it
 * @param msg the message the user entered
 */
void StompProtocol::processFromClient(string &msg) {
    for (size_t i = 1; i < commandFromClient.size(); ++i) {
        if (commandFromClient.at(i) == msg.substr(0, msg.find(' '))) {
            //uses switch and case by the content
            switch (i) {
                case 1: //login
                     login(msg);
                    break;
                case 2: //join
                     join(msg);
                    break;
                case 3: //exit
                     exit(msg);
                    break;
                case 4: //add
                     add(msg);
                    break;
                case 5: //borrow
                     borrow(msg);
                    break;
                case 6: //return
                     returnBook(msg);
                    break;
                case 7: //status
                     status(msg);
                    break;
                case 8: //logout
                     logout(msg);
                    break;
            }
        }
    }
}


/**
 * the server responsed that user is now connected
 * change the user's paramete to connected
 */
void StompProtocol::connected() {
    if(!user.isAboutToLogout())
    {
        user.setIsConnected(true);
        cout << "Login successful"<< endl;
    }
}

/**
 * handles all frame from server that begins with MESSAGE
 * @param msg the message the server sent
 * @return a message from the user to the server
 */
MessageFrame *StompProtocol::message(MessageFrame &msg) {
    //everything is done only if the user is not going to logout any moment
    if(!user.isAboutToLogout()) {
        string genre(msg.getHeader("destination"));
        cout << genre + ": " + msg.getBody() + '\n';

        //status message - to print all subscribed users's inventory
        if (msg.getBody().size() >= 6 && msg.getBody().find("status") != string::npos) {
            //creates a new message with the inventory content (in a specific genre)
            vector<pair<string, string>> returnMap = vector<pair<string, string>>();
            pair<string, string> p("destination", genre);
            returnMap.push_back(p);

            auto *msgFrame = new MessageFrame("SEND", returnMap, user.toStringStatus(genre));
            return msgFrame;
        }

        //return a book message - a user want to return a book he borrowed
        if ((msg.getBody().find(' ') != string::npos) &&
            (msg.getBody().substr(0, msg.getBody().find(' '))) == "Returning") {
            string name = msg.getBody().substr(msg.getBody().find_last_of(' ') + 1);

            //if the user should get back the book is 'me'
            if (name == user.getUserName()) {
                int len = msg.getBody().find_last_of(' ') - 3 - (msg.getBody().find_first_of(' ') + 1);
                string bookName = msg.getBody().substr(msg.getBody().find_first_of(' ') + 1, len);
                Book toReturnBook = Book(bookName, genre);
                user.addInventory(toReturnBook);
            }
            return nullptr;
        }

        //user want to borrow a book message
        if ((msg.getBody().find(' ') != string::npos) &&
            msg.getBody().substr(msg.getBody().find(' ') + 1, 4) == "wish") {
            string bookName = msg.getBody().substr(msg.getBody().find("borrow") + 7);

            //search if the user has in his inventory the book that another user want to borrow
            if(user.getInventory().find(genre)!=user.getInventory().end()) {
                for (auto &it: user.getInventory().at(genre)) {
                    if (it.first == bookName) {
                        //if he does, send a message that he can lend it to who wants
                        vector<pair<string, string>> returnMap = vector<pair<string, string>>();
                        pair<string, string> p("destination", genre);
                        returnMap.push_back(p);
                        string returnBody = user.getUserName() + " has " + bookName;
                        auto *msgFrame = new MessageFrame("SEND", returnMap, returnBody);
                        return msgFrame;
                    }
                }
            }
            return nullptr;
        }

        //Taking book from user message
        if ((msg.getBody().find(' ') != string::npos) &&
            (msg.getBody().substr(0, msg.getBody().find(' '))) == "Taking") {
            string name = msg.getBody().substr(msg.getBody().find_last_of(' ') + 1);

            //checks if someone said he is taking the book from 'me'
            if (name == user.getUserName()) {
                //if so, removes it from 'my' inventory
                int len = msg.getBody().find_last_of(' ') - 5 - (msg.getBody().find_first_of(' ') + 1);
                string bookName = msg.getBody().substr(msg.getBody().find_first_of(' ') + 1, len);
                user.removeInventory(genre, bookName);
            }
            return nullptr;
        }

        //add to inventory of client
        if (msg.getBody().size() >= 3 && msg.getBody().find("has") != string::npos) {
            if ((msg.getBody().size() < 9) || msg.getBody().find("has added") == string::npos) {
                //if someone has the book 'I' want to borrow
                string bookName = msg.getBody().substr(msg.getBody().find("has") + 4);
                string owner = msg.getBody().substr(0, msg.getBody().find_first_of(' '));
                for (size_t i = 0; i < user.getWantedBooks().size(); ++i) {
                    if (user.getWantedBooks().at(i) == bookName) {
                        //creates a new book and add it to my inventory and to the borrowed books
                        user.removeWantedBooks(bookName);
                        Book bookToAdd(Book(bookName, genre));
                        user.addInventory(bookToAdd);
                        user.addBorrowedBooks(bookToAdd, owner);

                        //creates a meesage says user is taking that book from someone else
                        vector<pair<string, string>> returnMap = vector<pair<string, string>>();
                        returnMap.emplace_back("destination", genre);
                        string body = "Taking " + bookName + " from " + owner;
                        auto *msgFrame = new MessageFrame("SEND", returnMap, body);
                        return msgFrame;
                    }
                }
            }
        }
        return nullptr;
    }
    return nullptr;
}

/**
 * the server sent a receipt for some message frame
 * @param msg the message the server sent
 */
void StompProtocol::receipt(MessageFrame &msg) {
    int id= stol(msg.getHeader("receipt-id"));
    //gets the original message the user sent and asked for a receipt, that removes it from the map
    MessageFrame originalMsg = user.getReceiptMap().at(id);

    //EED TO CALL BOOK DESTRUCTOR
    user.removeReceiptMap(id);

    //checks the original message and according to that knows what to so next
    if(originalMsg.getCommand() == "SUBSCRIBE")
    {
        //if it was for subscribing a topic, inform the user he is now subscribed
        string genre(originalMsg.getHeader("destination"));
        user.addSubscribedTo(id, genre);
        cout<<"joined club " + genre +'\n';
    }
    else if(originalMsg.getCommand() == "UNSUBSCRIBE")
    {
        //if it was for unsubscription a topic, inform the user he is not subscribed anymore
        int subId =  stol(originalMsg.getHeader("id"));
        string genre = user.getSubscribedTo().at(subId);
        user.removeSubscribeTo(subId);
        cout<<"Exit club " + genre +'\n';
    }
    else if(originalMsg.getCommand() == "DISCONNECT")
    {
        //if it was for login out - do that
        cout<<"user "<<user.getUserName()<<" has logged out\n";
        disconnect();
    }
}

/**
 * the the server sent an error for some message frame
 * @param msg
 */
void StompProtocol::error(MessageFrame &msg) {
    //if it was bad password - the user gets another chance
    if(msg.getBody() == "Wrong Password"){
        cout<<"wrong password - please try again \n";
    }
    //else, disconnect the user from the server
    else {
        cout<<"ERROR - "<<msg.getBody()<<endl;
        disconnect();
    }
}

/**
 * disconnects the user from the server
 */
void StompProtocol::disconnect() {
    user.setIsConnected(false);
    isDone = true;
}




/**
 * login message from user
 * @param msg the message the user typed
 */
void StompProtocol::login(string &msg) {
    //checks first user is connected to server
    if(!user.getIsConnected()) {
        //parse what user wrote
        string passcode = msg.substr(msg.find_last_of(' ') + 1);
        msg = msg.substr(0, msg.find_last_of(' '));
        string name = msg.substr(msg.find_last_of(' ') + 1);
        msg = msg.substr(0, msg.find_last_of(' '));
        string host = msg.substr(msg.find(' ') + 1);

        //creates a message with all parameters needd, for the server
        vector<pair<string,string>> returnMap = vector<pair<string,string>>();
        returnMap.emplace_back("accept-version","1.2");
        returnMap.emplace_back("host",host);
        returnMap.emplace_back("login",name);
        returnMap.emplace_back("passcode",passcode);
        auto *messageFrame = new MessageFrame("CONNECT",returnMap,"");

        //updates the user's data from what he typed
        user.setUserName(name);
        user.setPasscode(passcode);
        //send to server
        connectionHandler.sendMessageFrame(*messageFrame);
    }
    else
    {
        cout<<"User is already logged in\n";
    }
}

/**
 * user want to subscribe for a new topic
 * @param msg
 */
void StompProtocol::join(string &msg) {
    //checks first user is connected to server
    if(user.getIsConnected())
    {
        string genre(msg.substr(msg.find_first_of(' ')+1));

        //checks that the user is not subscribed
        for (auto& it: user.getSubscribedTo()) {
            if(it.second == genre){
                return;
            }
        }

        //generate new id's
        subscribeIdCounter++;
        receiptIdCounter++;
        //creates message to send
        vector<pair<string,string>> returnMap = vector<pair<string,string>>();
        returnMap.emplace_back("destination",genre);
        returnMap.emplace_back("id",to_string(subscribeIdCounter));
        returnMap.emplace_back("receipt",to_string(receiptIdCounter));
        auto *messageFrame = new MessageFrame("SUBSCRIBE", returnMap, "");
        MessageFrame tmpMessage(*messageFrame);
        //add the message to the user receipt map
        user.addReceiptMap(receiptIdCounter,tmpMessage);
        //send message to server
        connectionHandler.sendMessageFrame(*messageFrame);
    }
    else{
        cout<<"Error: User is not logged in \n";
    }
}

/**
 * the user want to exit from a topic he is subscribed to
 * @param msg the message the user typed
 */
void StompProtocol::exit(string &msg) {
    //checks first user is connected to server
    if(user.getIsConnected())
    {
        //gets the topic
        string genre(msg.substr(msg.find_first_of(' ')+1));
        int id(0);

        //gets the id the user subscribed with
        for (auto& it: user.getSubscribedTo()) {
            if(it.second == genre){
              id = it.first;
            }
        }
        if(id != 0){
            //calls the method that unsubscribe him
           unsubscribe(id);
        }
    }
    else{
        cout<<"Error: User is not logged in \n";
    }
}

/**
 * does the actual unsubscribe work
 * @param id the id the user sybscribed to that topic with
 */
void StompProtocol::unsubscribe(int id){
    //checks first user is connected to server
    if(user.getIsConnected()){
        //creates all data in order to send a frame to server
        receiptIdCounter++;
        vector<pair<string,string>> returnMap = vector<pair<string,string>>();;
        returnMap.emplace_back("id",to_string(id));
        returnMap.emplace_back("receipt",to_string(receiptIdCounter));

        //creates a new message frame
        auto *messageFrame = new MessageFrame("UNSUBSCRIBE", returnMap, "");
        MessageFrame tmpMessage(*messageFrame);

        //add the message to receipt mep - for future use
        user.addReceiptMap(receiptIdCounter,tmpMessage);
        //sends the message to server
        connectionHandler.sendMessageFrame(*messageFrame);
    }
}

/**
 * the user added a book to his inventory
 * @param msg the message the user typed
 */
void StompProtocol::add(string &msg) {
    //checks first user is connected to server
    if(user.getIsConnected()){
        msg = msg.substr(4);
        string genre = msg.substr(0, msg.find_first_of(' '));
        string book = msg.substr(msg.find_first_of(' ')+1);

        //check if the user doesn't have this book already
        if(user.getInventory().find(genre) != user.getInventory().end()){
            for (auto& it: user.getInventory().at(genre)){
                if (it.first == book){
                    return;
                }
            }
        }

        //creates a new book and add it to the user's inventory
        Book newBook(Book(book, genre));
        user.addInventory(newBook);
        cout<<"The book "<<book<< " was added to your inventory\n";

        //create a message to send
        vector<pair<string,string>> returnMap = vector<pair<string,string>>();;
        returnMap.emplace_back("destination",genre);
        string body = user.getUserName() + " has added the book " + book + '\n';
        auto *messageFrame = new MessageFrame("SEND", returnMap, body);
        //send message to server
        connectionHandler.sendMessageFrame(*messageFrame);
    }
    else{
        cout<<"Error: User is not logged in \n";
    }
}

/**
 * user want to borrow a book
 * @param msg the message the user typed
 */
void StompProtocol::borrow(string &msg) {
    //checks first user is connected to server
    if(user.getIsConnected()){
        msg = msg.substr(7);
        string genre = msg.substr(0, msg.find_first_of(' '));
        string book = msg.substr(msg.find_first_of(' ')+1);
        //checks that the user doesn't have the book alredy
        if(user.getInventory().find(genre)!= user.getInventory().end()) {
            for (auto &it: user.getInventory().at(genre)) {
                if (it.first == book) {
                    return;
                }
            }
        }
        //add that book to the vector of books the user want
        user.addWantedBooks(book);

        //creates message to send
        vector<pair<string,string>> returnMap = vector<pair<string,string>>();;
        returnMap.emplace_back("destination",genre);
        string body = user.getUserName() + " wish to borrow " + book + '\n';
        auto *messageFrame = new MessageFrame("SEND", returnMap, body);
        //send message to server
        connectionHandler.sendMessageFrame(*messageFrame);
    }
    else{
        cout<<"Error: User is not logged in \n";
    }
}

/**
 * return a book borrowed before
 * @param msg the message the user typed
 */
void StompProtocol::returnBook(string &msg) {
    //checks first user is connected to server
    if(user.getIsConnected()){
        msg = msg.substr(7);
        string genre = msg.substr(0, msg.find_first_of(' '));
        string book = msg.substr(msg.find_first_of(' ')+1);
        //search for the book in the borrowed map
        if(user.getBorrowedBooks().find(book) == user.getBorrowedBooks().end()){
            return;
        }

        //removes the book from the map of books the user borrowed, and from his inventory
        string owner(user.getBorrowedBookOwner(book));
        user.removeInventory(genre,book);
        user.removeBorrowedBooks(book);


        //creates message to send
        vector<pair<string,string>> returnMap = vector<pair<string,string>>();;
        returnMap.emplace_back("destination", genre);
        string body = "Returning " + book + " to " + owner +'\n';
        auto *messageFrame = new MessageFrame("SEND", returnMap, body);
        //send message to server
        connectionHandler.sendMessageFrame(*messageFrame);
    }
    else{
        cout<<"Error: User is not logged in \n";
    }
}

/**
 * user want to see all books in a specific topic
 * @param msg the massage the user wrote
 */
void StompProtocol::status(string &msg) {
    //checks first user is connected to server
    if(user.getIsConnected()) {
        //creates a message to send
        string genre = msg.substr(msg.find_first_of(' ') + 1);
        vector<pair<string,string>> returnMap = vector<pair<string,string>>();;
        returnMap.emplace_back("destination", genre);
        auto *messageFrame = new MessageFrame("SEND",returnMap, "book status");
        //send message to server
        connectionHandler.sendMessageFrame(*messageFrame);
    }
    else{
        cout<<"Error: User is not logged in \n";
    }
}

/**
 * a user want to logout from the server
 * @param msg the message the user wrote
 */
void StompProtocol::logout(string &msg) {
    //checks first user is connected to server
    if(user.getIsConnected()) {
        //sets that the user wants to logout so nothing will be sent to him
        user.setAboutToLogout(true);
        //calls the unsubscribe for all topics
        for (auto& it: user.getSubscribedTo()) {
            unsubscribe(it.first);
        }

        //creates a message to send server
        vector<pair<string,string>> returnMap = vector<pair<string,string>>();
        receiptIdCounter++;
        returnMap.emplace_back("receipt", to_string(receiptIdCounter));
        auto *messageFrame = new MessageFrame("DISCONNECT",returnMap, "");
        MessageFrame tmpMessage(*messageFrame);
        user.addReceiptMap(receiptIdCounter,tmpMessage);
        //send message to server
        connectionHandler.sendMessageFrame(*messageFrame);
    }
    else
    {
        cout<<"Error: User is not logged in \n";
    }
}

//destructor
StompProtocol::~StompProtocol() {
    commandFromServer.clear();
    commandFromClient.clear();
}
