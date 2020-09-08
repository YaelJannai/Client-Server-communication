# Client-Server-communication
Java & C++

Implementation of a book club. Users will be able to signup for reading clubs and borrow books from each other.
Implement of both a server, which will provide STOMP server services, and a client, which a user can use in order to interact with the rest of the users. 
The server will be implemented in Java and will support both Thread-Per-Client (TPS) and the Reactor, choosing which one according to arguments given on startup. 
The client will be implemented in C++ , and will hold the required logic as described below.
All communication between the clients and the server will be according to the STOMP `Simple-Text-OrientedMessaging-Protocol' protocol.
