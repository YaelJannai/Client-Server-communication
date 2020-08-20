package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

import java.util.HashMap;
import java.util.LinkedHashMap;


public class StompMessagingProtocolImpl implements StompMessagingProtocol<MessageFrame> {

    private boolean shouldTerminate;
    private Connections<MessageFrame> connections;
    private int connectionId = 0;
    private StompUser user;

    /**
     * constructor of the messaging protocol, resetting al the fields
     */
    public StompMessagingProtocolImpl() {
        shouldTerminate = false;
        this.user = new StompUser();
    }

    /**
     * Used to initiate the current client protocol with it's personal connection ID and the connections implementation
     *
     * @param connectionId the id of the ConnectionHandler that has this protocol
     * @param connections the Connection object that is in charge of all the ConnectionHandlers
     */
    @Override
    public void start(int connectionId, Connections<MessageFrame> connections) {
        this.connections = connections;
        this.connectionId = connectionId;
        //until the user does not send a CONNECT frame we cannot know which user it is, but we do know it is not connected.
        user.setConnected(false);
    }

    /**
     * process the given message
     * @param message the received message
     * handles the message per case.
     */
    @Override
    public void process(MessageFrame message) {
        //we switch between the different commands to know what to do.
        if (message != null)
        {
            String command = message.getCommand();
            switch (command){
                case "CONNECT":
                    connect(message);
                    break;
                case "SEND":
                    send(message);
                    break;
                case "SUBSCRIBE":
                    subscribe(message);
                    break;
                case "UNSUBSCRIBE":
                    unsubscribe(message);
                    break;
                case "DISCONNECT":
                    disconnect(message);
                    break;
                default:
                    commandError(message);
            }
        }
    }


    /**
     * @return true if the connection should be terminated
     */
    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    /**
     * This method is called if the user want to connect
     * @param message the message that the client has sent.
     */
    private void connect(MessageFrame message){

        //check if the userName that the user gave when he tried to connect is already exists
        if(connections.getUserMap().containsKey(message.getHeaders().get("login"))) {
            //if it does, we get the user from the usersMap in the connections
           StompUser tmpUser = connections.getUserMap().get(message.getHeaders().get("login"));

           //if the user is already connected from another client we send an error
           if(tmpUser.isConnected())
           {
               LinkedHashMap<String , String> returnMap = new LinkedHashMap<>();
               returnMap.put("message", "Connection Error");
               returnMap.put("user-name", message.getHeaders().get("login"));
               returnMap.put("passcode", message.getHeaders().get("passcode"));
               String body = "the user "+ message.getHeaders().get("login")+ " is already logged in";
               MessageFrame errorMsg =  new MessageFrame("ERROR", returnMap, body);
               connections.send(connectionId,errorMsg);
           }
           //if the passcode that the user gave is correct
           else if(tmpUser.getPasscode().equals(message.getHeaders().get("passcode")))
           {
               //we set the user of the protocol to be the user from the usersMap, and now set that he is connected
               this.user = tmpUser;
               this.user.setConnected(true);

               //send a CONNECTED frame back to the client.
               LinkedHashMap<String , String> returnMap = new LinkedHashMap<>();
               returnMap.put("version", "1.2");
               MessageFrame returnMsg = new MessageFrame("CONNECTED", returnMap, "");
               connections.send(connectionId,returnMsg);

               //if the client asked for a receipt, we send it a RECEIPT frame.
               if(message.getHeaders().containsKey("receipt"))
               {
                   LinkedHashMap<String , String> returnMap2 = new LinkedHashMap<>();
                   returnMap2.put("receipt-id",message.getHeaders().get("receipt"));
                   MessageFrame receiptMsg =  new MessageFrame("RECEIPT", returnMap2, "");
                   connections.send(connectionId,receiptMsg);
               }
           }
           //if the passcode that the user gave is incorrect, we send error
           else
           {
               LinkedHashMap<String , String> returnMap = new LinkedHashMap<>();
               returnMap.put("message", "Connection Error");
               returnMap.put("user-name", message.getHeaders().get("login"));
               returnMap.put("passcode", message.getHeaders().get("passcode"));
               MessageFrame errorMsg =  new MessageFrame("ERROR", returnMap, "Wrong Password");
               connections.send(connectionId,errorMsg);
           }

        }
        //if the user name doesn't exist in the usersMap - it is a new user
        else
        {
            //we set the user of the protocol to be the new user, and ask the connections to add him to its usersMap
            user.setConnected(true);
            user.setUserName(message.getHeaders().get("login"));
            user.setPasscode(message.getHeaders().get("passcode"));
            connections.addUserMap(user);

            //send a CONNECTED frame back to the client.
            LinkedHashMap<String , String> returnMap = new LinkedHashMap<>();
            returnMap.put("version", "1.2");
            MessageFrame returnMsg = new MessageFrame("CONNECTED", returnMap, "");
            connections.send(connectionId,returnMsg);

            //if the client asked for a receipt, we send it a RECEIPT frame.
            if(message.getHeaders().containsKey("receipt"))
            {
                LinkedHashMap<String , String> returnMap2 = new LinkedHashMap<>();
                returnMap2.put("receipt-id",message.getHeaders().get("receipt"));
                MessageFrame receiptMsg =  new MessageFrame("RECEIPT", returnMap2, "");
                connections.send(connectionId,receiptMsg);
            }
        }
    }

    /**
     * This method is called if the user want to send a message to a group of users.
     * @param message the message that the client has sent.
     */
    private void send (MessageFrame message){
        if(user.isConnected()) {
            if (message.getHeaders().containsKey("destination")) {
                if (connections.getTopicsMap().containsKey(message.getHeaders().get("destination"))) {
                    if (!message.getBody().equals("")) {

                        //sending the message to all the subscribed clients
                        LinkedHashMap<String, String> returnMap = new LinkedHashMap<String, String>();
                        returnMap.put("subscription", "0");
                        returnMap.put("destination", message.getHeaders().get("destination"));
                        MessageFrame returnMsg = new MessageFrame("MESSAGE", returnMap, message.getBody());
                        connections.send(message.getHeaders().get("destination"), returnMsg);

                        //if the client asked for a receipt, we send it a RECEIPT frame.
                        if (message.getHeaders().containsKey("receipt")) {
                            LinkedHashMap<String, String> returnMap2 = new LinkedHashMap<>();
                            returnMap2.put("receipt-id", message.getHeaders().get("receipt"));
                            MessageFrame receiptMsg = new MessageFrame("RECEIPT", returnMap2, "");
                            connections.send(connectionId, receiptMsg);
                        }
                    }
                }
            }
        }
        //if the user is not connected, he cannot do this kind of command, so sending an ERROR frame
        else {
            LinkedHashMap<String , String> returnMap = new LinkedHashMap<>();
            returnMap.put("message", "Connection Error");
            returnMap.put("destination", message.getHeaders().get("destination"));
            notConnectedError(message,returnMap);
        }
    }

    /**
     * This method is called if the user want to subscribe to a topic
     * @param message the message that the client has sent.
     */
    private void subscribe(MessageFrame message){
        if(user.isConnected()) {
            if (message.getHeaders().containsKey("destination")) {
                if (message.getHeaders().containsKey("id")) {

                    //subscribe the user if it can, if no  do nothing
                    if(connections.subscribe(message.getHeaders().get("destination"),Integer.parseInt(message.getHeaders().get("id")),connectionId))
                    {
                        //if the client asked for a receipt, we send it a RECEIPT frame.
                        if (message.getHeaders().containsKey("receipt")) {
                            LinkedHashMap<String, String> returnMap2 = new LinkedHashMap<>();
                            returnMap2.put("receipt-id", message.getHeaders().get("receipt"));
                            MessageFrame receiptMsg = new MessageFrame("RECEIPT", returnMap2, "");
                            connections.send(connectionId, receiptMsg);
                        }
                    }
                }
            }
        }
        //if the user is not connected, he cannot do this kind of command, so sending an ERROR frame
        else {
            LinkedHashMap<String , String> returnMap = new LinkedHashMap<>();
            returnMap.put("message", "Connection Error");
            returnMap.put("id", message.getHeaders().get("id"));
            notConnectedError(message, returnMap);
        }

    }


    /**
     * This method is called if the user want to unsubscribe from a topic
     * @param message the message that the client has sent.
     */
    private void unsubscribe(MessageFrame message){
        if(user.isConnected()) {
            if (message.getHeaders().containsKey("id")) {
                //unsubscribe the user if it can, if no  do nothing
               if(connections.unsubscribe(connectionId, Integer.parseInt(message.getHeaders().get("id")))) {
                   //if the client asked for a receipt, we send it a RECEIPT frame.
                   if (message.getHeaders().containsKey("receipt")) {
                       LinkedHashMap<String, String> returnMap2 = new LinkedHashMap<>();
                       returnMap2.put("receipt-id", message.getHeaders().get("receipt"));
                       MessageFrame receiptMsg = new MessageFrame("RECEIPT", returnMap2, "");
                       connections.send(connectionId, receiptMsg);
                   }
               }
            }
        }
        //if the user is not connected, he cannot do this kind of command, so sending an ERROR frame
        else {
            LinkedHashMap<String , String> returnMap = new LinkedHashMap<>();
            returnMap.put("message", "Connection Error");
            returnMap.put("id", message.getHeaders().get("id"));
            notConnectedError(message, returnMap);
        }

    }

    /**
     * This method is called if the user want to disconnect from the server
     * @param message the message that the client has sent.
     */
    private void disconnect(MessageFrame message){
        if(user.isConnected()) {
            //sending the RECEIPT frame.
            if (message.getHeaders().containsKey("receipt")) {

                //setting the user to be not connected, and the shouldTerminate boolean to be true.
                user.setConnected(false);
                shouldTerminate = true;

                LinkedHashMap<String, String> returnMap = new LinkedHashMap<>();
                returnMap.put("receipt-id", message.getHeaders().get("receipt"));
                MessageFrame receiptMsg = new MessageFrame("RECEIPT", returnMap, "");
                connections.send(connectionId, receiptMsg);
            }
        }
        //if the user is not connected, he cannot do this kind of command, so sending an ERROR frame
        else {
            LinkedHashMap<String , String> returnMap = new LinkedHashMap<>();
            returnMap.put("message", "Connection Error");
            returnMap.put("id", message.getHeaders().get("receipt-id"));
            notConnectedError(message, returnMap);
        }
    }

    /**
     * This method is called if the client gave an unknown command - not suppose to happen.
     * @param message the message that the client has sent.
     */
    private void commandError(MessageFrame message)
    {
        //send an ERROR frame.
        LinkedHashMap<String , String> returnMap = new LinkedHashMap<>();
        returnMap.put("message", "No such command");
        String errorBody = "The message:\n-----";
        errorBody += message.toString();
        errorBody += "\n-----\n Did not contain a valid command.";
        MessageFrame errorMsg =  new MessageFrame("ERROR", returnMap, errorBody);
        connections.send(connectionId,errorMsg);
    }


    private void notConnectedError(MessageFrame message, LinkedHashMap<String,String> returnMap) {
        MessageFrame errorMsg =  new MessageFrame("ERROR", returnMap, "User is not logged in");
        connections.send(connectionId,errorMsg);
    }
}
