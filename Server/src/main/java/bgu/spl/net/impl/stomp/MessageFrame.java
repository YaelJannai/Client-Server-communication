package bgu.spl.net.impl.stomp;

import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Constructor of the Frame.
 */
public class MessageFrame {

    private String command;
    private LinkedHashMap<String, String> headers;
    private String body;

    public MessageFrame(String command, LinkedHashMap<String, String> headers, String body){
        this.command = command;
        this.headers = headers;
        this.body = body;
    }

    /**
     * getter for the command.
     * @return the command.
     */
    public String getCommand() {
        return command;
    }

    /**
     * getter for the Headers.
     * @return the headers.
     */
    public LinkedHashMap<String, String> getHeaders() {
        return headers;
    }

    /**
     * getter for the Body.
     * @return the body of it.
     */
    public String getBody() {
        return body;
    }

    /**
     * setter for the headers of the message.
     * @param key - the name of the field.
     * @param value - the content of a field.
     */
    public void setHeaderByKey(String key, String value)
    {
        headers.putIfAbsent(key,value);
    }

    /**
     * convert the Frame content to a string.
     * @return a string representing the messge Frame.
     */
    @Override
    public String toString() {
        StringBuilder str = new StringBuilder(command + '\n');
        for (Map.Entry<String, String> entry : headers.entrySet()) {
            str.append(entry.getKey()).append(':').append(entry.getValue()).append('\n');
        }
        str.append('\n');
        str.append(body);
        return str.toString();
    }
}
