package bgu.spl.net.impl.stomp;


import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {

        Connections<MessageFrame> connections = new ConnectionsImpl();
        short port = Short.parseShort(args[0]);

        if(args[1].equals("tpc")) {
            Server.threadPerClient(
                    port, //port
                    StompMessagingProtocolImpl::new, //protocol factory
                    StompEncoderDecoder::new, //message encoder decoder factory
                    connections //connections
            ).serve();
        }
        else if(args[1].equals("reactor")) {
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(),
                    port, //port
                    StompMessagingProtocolImpl::new, //protocol factory
                    StompEncoderDecoder::new, //message encoder decoder factory
                    connections //connections
            ).serve();
        }
    }
}
