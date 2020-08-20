package bgu.spl.net.srv;


import bgu.spl.net.impl.stomp.MessageFrame;
import bgu.spl.net.impl.stomp.StompUser;

import java.util.HashMap;
import java.util.Map;

public class ConnectionsImpl implements Connections<MessageFrame> {

    private HashMap<Integer, ConnectionHandler<MessageFrame>> clientMap;
    private HashMap<String, HashMap<Integer, Integer>> topicsMap;
    private HashMap<String, StompUser> userMap;
    private int messageId;

    /**
     * The constructor of the class, creating new maps.
     */
    public ConnectionsImpl()
    {
        clientMap = new HashMap<>();
        topicsMap = new HashMap<>();
        userMap = new HashMap<>();
        messageId = 1;
    }

    /**
     * This method is for the Reactor/BaseServer to send their ConnectionHandler and their matching ids,
     * so the Connection could map them.
     * @param id the connectionId of the ConnectionHandler.
     * @param handler the connectionHandler itself.
     */
    public void insertHandler(int id, ConnectionHandler<MessageFrame> handler)
    {
        clientMap.putIfAbsent(id,handler);
    }

    /**
     * This method is responsible to send a message to a specific client.
     * @param connectionId the connectionId that matches the client
     * @param msg the message to be send
     * @return true - if the message has been sent, false - else.
     */
    @Override
    public boolean send(int connectionId, MessageFrame msg) {
        if (clientMap.containsKey(connectionId)){
            clientMap.get(connectionId).send(msg);
            return true;
        }
        return false;
    }

    /**
     * This method is responsible to send a message to a group of client.
     * @param channel the group of client to get the message.
     * @param msg the message to be send.
     */
    @Override
    public void send(String channel, MessageFrame msg) {

        msg.setHeaderByKey("Message-id", String.valueOf(messageId));
        messageId++;

        if (topicsMap.containsKey(channel)){
            //we go over all the subscribers to the channel
            for (Map.Entry<Integer,Integer> entry: topicsMap.get(channel).entrySet()) {
                //for each one we create a new MessageFrame from the already given one, and change the 'subscription' header to match the subscriberId.
                MessageFrame myMsg = new MessageFrame(msg.getCommand(),msg.getHeaders(),msg.getBody());
                myMsg.setHeaderByKey("subscription", String.valueOf(entry.getValue()));
                clientMap.get(entry.getKey()).send(myMsg);
            }
        }
    }

    /**
     * This method is in charge to disconnect the connected user
     * @param connectionId the id of the ConnectionHandler that was in charge of this client
     */
    @Override
    public void disconnect(int connectionId) {

        //checking if the user has unsubscribed  from all of its topics
        for (Map.Entry<String, HashMap<Integer, Integer>> entry : topicsMap.entrySet()) {

            if (entry.getValue().containsKey(connectionId)) {
                unsubscribe(connectionId,entry.getValue().get(connectionId));
            }
        }
        clientMap.remove(connectionId);
    }

    /**
     * This method is in charge to subscribe a user to a specific topic(channel).
     * @param topic the topic that user is about to be subscribe to
     * @param id the subscriptionId of the user to this topic.
     * @param connectionId the connectionId that match the client that this user is connected on now.
     * @return true - if the subscribing has been successful return true, else return false.
     */
    public boolean subscribe(String topic, int id, int connectionId)
    {
        //first running over all the topics in the topicMap to check that the pair <connectionId, id> is unique.
        for (Map.Entry<String, HashMap<Integer, Integer>> entry: topicsMap.entrySet()) {
            for (Map.Entry<Integer,Integer> client: topicsMap.get(entry.getKey()).entrySet()) {
                if(client.getKey() == connectionId && client.getValue() == id)
                {
                    //if it is not unique, we do not subscribe
                    return false;
                }
            }
        }

        //now we check if this topic is already exists
        if(!topicsMap.containsKey(topic))
        {
            //if not - we create a new topic with the given name of topic, and later subscribe the user to it.
            HashMap<Integer,Integer> subscribersMap = new HashMap<>();
            topicsMap.putIfAbsent(topic,subscribersMap);
            topicsMap.get(topic).putIfAbsent(connectionId,id);
            return true;
        }

        //if the topic is already exist
        //we check if this specific user is already subscribed to this topic with another subscriptionId.
        //if not - we subscribe him and returning true, else do nothing and returning false.
        return topicsMap.get(topic).putIfAbsent(connectionId, id) == null;
    }

    /**
     * This method is in charge to unsubscribe a user from a specific topic
     * @param connectionId the connectionId of the ConnectionHandler that works with the client that the user is connected on.
     * @param id - the subscriptionId that this user is subscribe with to the topic he want to exit.
     * @return true - if the unsubscribe has been successful return true, else return false.
     */
    public boolean unsubscribe(int connectionId, int id)
    {
        //first we run over all the topics in the topicMap
        for (Map.Entry<String, HashMap<Integer, Integer>> entry : topicsMap.entrySet()) {

            //then, we check if the user is subscribed to this topic
            if(entry.getValue().containsKey(connectionId))
            {
                //last, we check if the user is subscribed to this topic with the given subscriptionId.
               if(entry.getValue().get(connectionId) == id)
               {
                   //if it does, so unsubscribe him.
                   entry.getValue().remove(connectionId);
                   return true;
               }
            }
        }

        return false;
    }

    /**
     * Getter for the TopicMap
     * @return the topicMap
     */
    public HashMap<String, HashMap<Integer, Integer>> getTopicsMap() {
        return topicsMap;
    }

    /**
     * Getter for the UserMap
     * @return the userMap
     */
    public HashMap<String, StompUser> getUserMap() {
        return userMap;
    }

    /**
     * Add user to the user map
     * @param user the user to be added
     */
    public void addUserMap(StompUser user)
    {
        this.userMap.putIfAbsent(user.getUserName(),user);
    }
}
