package bgu.spl.net.impl.stomp;

import java.util.HashMap;

public class StompUser {

    private String userName;
    private String passcode;
    private boolean isConnected;

    public StompUser() {
        this.userName = "";
        this.passcode = "";
        this.isConnected = false;
    }

    public String getUserName() {
        return userName;
    }

    public void setUserName(String userName) {
        this.userName = userName;
    }

    public String getPasscode() {
        return passcode;
    }

    public void setPasscode(String passcode) {
        this.passcode = passcode;
    }

    public boolean isConnected() {
        return isConnected;
    }

    public void setConnected(boolean connected) {
        isConnected = connected;
    }
}
