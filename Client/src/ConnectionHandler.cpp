#include <ConnectionHandler.h>
#include <mutex>

using boost::asio::ip::tcp;

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

using namespace std;

mutex ConnectionHandler::mutexHandler;

//constructor
ConnectionHandler::ConnectionHandler(string host, short port, StompEncoderDecoder encdec): host_(host), port_(port), io_service_(), socket_(io_service_), encdec(encdec){}

//destructor
ConnectionHandler::~ConnectionHandler() {
    close();
}

/**
 * connect to the remote machine
 * @return if could connect or not
 */
bool ConnectionHandler::connect() {
    std::cout << "Starting connect to " 
        << host_ << ":" << port_ << std::endl;
    //trues to establish a connection to a given host, if cannot, throws an exception
    try {
		tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
		boost::system::error_code error;
		socket_.connect(endpoint, error);
		if (error)
			throw boost::system::system_error(error);
    }
    catch (std::exception& e) {
        std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

/**
 * Read a fixed number of bytes from the server - blocking.
 * Returns false in case the connection is closed before bytesToRead bytes can be read.
 * @param bytes an array to write the bytes the function read
 * @param bytesToRead the number of bytes to red
 * @return if could read or not
 */
bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
    size_t tmp = 0;
    boost::system::error_code error;
    try {
        //while there is no error and the number of bytes to read is positive
        while (!error && bytesToRead > tmp ) {
            //read bytes
            tmp += socket_.read_some(boost::asio::buffer(bytes+tmp, bytesToRead-tmp), error);
        }
        if(error)
            throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

/**
 * Send a fixed number of bytes from the client - blocking.
 * Returns false in case the connection is closed before all the data is sent.
 * @param bytes the bytes need to send
 * @param bytesToWrite the number of bytes to write
 * @return
 */
bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
    int tmp = 0;
	boost::system::error_code error;
	//locks the sending to be sure it sends everything as expected
    lock_guard<mutex> lockGuard(mutexHandler);
	try {
        //while there is no error and the number of bytes to write is positive
        while (!error && bytesToWrite > tmp ) {
            //write bytes
			tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
        }
		if(error)
			throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

/**
 * Read an ascii line from the server
 * Returns false in case connection closed before a newline can be read.
 * @param line - the line wants to read
 * @return if could read or not
 */
bool ConnectionHandler::getLine(std::string& line) {
    //calls a function that will deal with the actual reading of the line
    return getFrameAscii(line, '\n');
}

/**
 * Send an ascii line from the server
 * Returns false in case connection closed before all the data is sent.
 * @param line the line wants to write
 * @return if could write or not
 */
bool ConnectionHandler::sendLine(std::string& line) {
    //calls a function that will deal with the actual writing of the line
    return sendFrameAscii(line, '\n');
}
 
/**
 * Get Ascii data from the server until the delimiter character
 * Returns false in case connection closed before null can be read.
 * @param frame - the frame to read
 * @param delimiter - the char represents when to stop reading
 * @return if could read or not
 */
bool ConnectionHandler::getFrameAscii(std::string& frame, char delimiter) {
    char ch;
    // Stop when we encounter the null character.
    // Notice that the null character is not appended to the frame string.
    try {
        do{
            if(!getBytes(&ch, 1))
            {
                return false;
            }
            if(ch!='\0')
                frame.append(1, ch);
        }while (delimiter != ch);
    } catch (std::exception& e) {
        std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}
 
/**
 * Send a message to the remote host.
 * Returns false in case connection is closed before all the data is sent.
 * @param frame  - the frame to send the server
 * @param delimiter the char represents when to stop writing
 * @return if could write or not
 */
bool ConnectionHandler::sendFrameAscii(const std::string& frame, char delimiter) {
	//calls another function to get the bytes from the frame and send them
    bool result=sendBytes(frame.c_str(),frame.length());
	if(!result) return false;
	//if could send, calls it again to know when to stop
	return sendBytes(&delimiter,1);
}

/**
 * Send a message to the remote host.
 * Returns false in case connection is closed before all the data is sent.
 * @param frame - the frame to write to the server
 * @return if could write or not
 */
bool ConnectionHandler::sendMessageFrame(MessageFrame &frame) {
    auto *bytes = new vector<char>(encdec.encode(frame));
    const short vecSize = bytes->size();
    char bytesArr[vecSize];

    for (int i = 0; i < vecSize; ++i) {
        bytesArr[i] = bytes->at(i);
    }
    bytes->clear();
    delete(bytes);
    bytes = nullptr;
    return sendBytes(bytesArr, vecSize);
}

/**
 * Close down the connection properly.
 */
void ConnectionHandler::close() {
    try{
        socket_.close();
    } catch (...) {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}


