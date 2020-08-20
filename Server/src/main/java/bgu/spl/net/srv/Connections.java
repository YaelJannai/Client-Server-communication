package bgu.spl.net.srv;

import bgu.spl.net.impl.stomp.MessageFrame;
import bgu.spl.net.impl.stomp.StompUser;

import java.io.IOException;
import java.util.HashMap;

public interface Connections<T> {

    void insertHandler(int id, ConnectionHandler<T> handler);

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    boolean subscribe(String topic, int id, int connectionId);

    boolean unsubscribe(int connectionId, int id);

    HashMap<String, HashMap<Integer, Integer>> getTopicsMap();

    HashMap<String, StompUser> getUserMap();

    void addUserMap(StompUser user);
}
