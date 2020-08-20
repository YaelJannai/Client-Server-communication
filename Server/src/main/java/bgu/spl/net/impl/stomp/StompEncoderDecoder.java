package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;

import java.awt.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.Arrays;


public class StompEncoderDecoder implements MessageEncoderDecoder <MessageFrame> {

    private String command = "";
    private LinkedHashMap<String,String> headers = new LinkedHashMap<String, String>();
    private String body = "";
    private byte[] byteArray = new byte[1 << 10];
    private int len = 0;
    private boolean wasEnter = false;
    private boolean doubleEnter = false;
    private String key = "";
    private String value = "";


    /**
     * add the next byte to the decoding process
     *
     * @param nextByte the next byte to consider for the currently decoded message
     * @return a message if this byte completes one or null if it doesn't.
     */
    @Override
    public MessageFrame decodeNextByte(byte nextByte) {
        //first we check if the  next byte is the last one
        if (nextByte == '\u0000') {
            //we take all the bytes that remain in the byteArray and create from it a string that will be the body of the message.
            body = new String(byteArray,0,len-1, StandardCharsets.UTF_8);
            len = 0;

            //create a new LinkedHashMap that will contains all the headers of the message.
            LinkedHashMap<String,String> tmpMap = new LinkedHashMap<String,String>();
            for (Map.Entry<String, String> entry : headers.entrySet())
            {
                tmpMap.put(entry.getKey(), entry.getValue());
            }

            //creating a new Message frame from the command we gor, the headers and the body.
            MessageFrame messageFrame = new MessageFrame(command,tmpMap,body);

            //clear all the fields for the message to decode.
            command = "";
            headers.clear();
            body = "";
            doubleEnter = false;
            byteArray = new byte[1 << 10];

            return messageFrame;
        }

        //second, we check if the command hasn't been set yet
        else if(command.equals("")) {
            //if so, if we got to the newLine char we set the command to be the string that is made from the bytes in the bytesArray, and resetting it.
            if (nextByte == '\n') {
                command = new String(byteArray,0,len, StandardCharsets.UTF_8);
                len = 0;
            }
            //if no, pushing the byte to the bytesArray
            else {
                pushByte(nextByte);
            }
        }
        //if we didn't got a double newLines - means we are in the headers
        else if(!doubleEnter) {
            //if the byte is not a newLine nor ':', so we in the key part of the header
            if(nextByte != '\n' && nextByte != ':') {
                wasEnter = false;
                pushByte(nextByte);
            }
            //if the byte is ':' so we finish the key part,
            //so we set the key to be the string that is made from the bytes in the bytesArray, and resetting it.
            else if(nextByte == ':') {
                wasEnter = false;
                key = new String(byteArray,0,len, StandardCharsets.UTF_8);
                len = 0;
            }
            //if the byte is newLine
            else {
                //if there was already an enter before, we set the boolean doubleEnter to be true, and from now on we add bytes for the body part of the message.
                if (wasEnter) {
                    wasEnter = false;
                    doubleEnter = true;
                }
                //if there was only one newLine char, we just finish the value part of the header
                else {
                    //set the wasEnter to be true
                    wasEnter = true;
                   // we set the value to be the string that is made from the bytes in the bytesArray, and resetting it.
                    value = new String(byteArray,0,len, StandardCharsets.UTF_8);
                    len = 0;
                    //adding the new header (key, value) to the headersMap, and resetting the key and the value.
                    this.headers.putIfAbsent(key,value);
                    key = "";
                    value = "";
                }
            }
        }
        //if there was doubleEnter before, we add bytes to the body.
        else {
            pushByte(nextByte);
        }
        return null;
    }

    private void pushByte(byte nextByte) {
        if (len >= byteArray.length) {
            byteArray = Arrays.copyOf(byteArray, len * 2);
        }

        byteArray[len++] = nextByte;
    }

    /**
     * encodes the given message to bytes array
     *
     * @param message the message to encode
     * @return the encoded bytes
     */
    @Override
    public byte[] encode(MessageFrame message) {
        byte[] bytesMessage = new byte[1 << 10];
        int pointer = 0;
        //adding the command part of the message first to the bytesArray
        byte[] command = message.getCommand().getBytes(StandardCharsets.UTF_8);
        for (byte b : command) {
            bytesMessage[pointer] = b;
            pointer++;
        }
        bytesMessage[pointer] = '\n';
        pointer++;

        LinkedHashMap<String,String> headers = message.getHeaders();
        //going over all of the headers and adding the key and the value separated with ':' char.
        for (Map.Entry<String, String> entry : headers.entrySet()) {
            byte[] key = entry.getKey().getBytes(StandardCharsets.UTF_8);
            byte[] value = entry.getValue().getBytes(StandardCharsets.UTF_8);
            for (byte b : key) {
                bytesMessage[pointer] = b;
                pointer++;
            }
            bytesMessage[pointer] = ':';
            pointer++;
            for (byte b : value) {
                bytesMessage[pointer] = b;
                pointer++;
            }
            bytesMessage[pointer] = '\n';
            pointer++;
        }
        bytesMessage[pointer] = '\n';
        pointer++;
        //adding the body part of the message after 2 newLines
        byte[] body = message.getBody().getBytes(StandardCharsets.UTF_8);
        for (byte b : body) {
            bytesMessage[pointer] = b;
            pointer++;
        }
        bytesMessage[pointer] = '\n';
        pointer++;
        //finishing the bytesArray with the null char.
        bytesMessage[pointer] = '\u0000';
        pointer++;
        byte[] shortBytesMessage = new byte[pointer];
        System.arraycopy(bytesMessage, 0, shortBytesMessage, 0, pointer);
        return shortBytesMessage;
    }
}
