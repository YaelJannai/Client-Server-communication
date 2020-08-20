package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.StompMessagingProtocol;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.ClosedSelectorException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.function.Supplier;

public class Reactor<T> implements Server<T> {

    private final int port;
    private final Supplier<StompMessagingProtocol<T>> protocolFactory;
    private final Supplier<MessageEncoderDecoder<T>> readerFactory;
    private final Connections<T> connections;
    private final ActorThreadPool pool;
    private Selector selector;
    private int idCounter;

    private Thread selectorThread;
    private final ConcurrentLinkedQueue<Runnable> selectorTasks = new ConcurrentLinkedQueue<>();

    /**
     * The constructor of the reactor, creates it and initialize it's fields.
     * @param numThreads number of threads the reactor will use during it's run.
     * @param port the server will listen to.
     * @param protocolFactory the protocol of the server to use.
     * @param readerFactory to encode message it sends and decode messages it receives.
     * @param connections the connection that will do the actual work with the client.
     */
    public Reactor(
            int numThreads,
            int port,
            Supplier<StompMessagingProtocol<T>> protocolFactory,
            Supplier<MessageEncoderDecoder<T>> readerFactory,
            Connections<T> connections) {

        this.pool = new ActorThreadPool(numThreads);
        this.port = port;
        this.protocolFactory = protocolFactory;
        this.readerFactory = readerFactory;
        this.connections = connections;
        idCounter = 0;
    }

    /**
     * The main loop of the server, Starts listening and handling new clients.
     */
    @Override
    public void serve() {
	selectorThread = Thread.currentThread();
	    // try to open connection.
        try (Selector selector = Selector.open();
                ServerSocketChannel serverSock = ServerSocketChannel.open()) {

            this.selector = selector; //just to be able to close

            // configure the connection and start it.
            serverSock.bind(new InetSocketAddress(port));
            serverSock.configureBlocking(false);
            serverSock.register(selector, SelectionKey.OP_ACCEPT);
			System.out.println("Server started");

			// the main loop of the reactor, will run until the flag will change.
            while (!Thread.currentThread().isInterrupted()) {

                selector.select();
                runSelectionThreadTasks();

                for (SelectionKey key : selector.selectedKeys()) {

                    if (!key.isValid()) {
                        continue;
                    } else if (key.isAcceptable()) {
                        handleAccept(serverSock, selector);
                    } else {
                        handleReadWrite(key);
                    }
                }

                selector.selectedKeys().clear(); //clear the selected keys set so that we can know about new events

            }

        } catch (ClosedSelectorException ex) {
            //do nothing - server was requested to be closed
        } catch (IOException ex) {
            //this is an error
            ex.printStackTrace();
        }

        System.out.println("server closed!!!");
        pool.shutdown();
    }

    /**
     *
     * @param chan the socket
     * @param ops
     */
    /*package*/ void updateInterestedOps(SocketChannel chan, int ops) {
        final SelectionKey key = chan.keyFor(selector);
        if (Thread.currentThread() == selectorThread) {
            key.interestOps(ops);
        } else {
            selectorTasks.add(() -> {
                if(key.isValid()) {
                    key.interestOps(ops);
                }
            });
            selector.wakeup();
        }
    }

    /**
     * the function that accepts the connection so that with could receive and send messages.
     * @param serverChan the socket that will handle the accept.
     * @param selector
     * @throws IOException - in case there will be an IO problem throws error.
     */
    private void handleAccept(ServerSocketChannel serverChan, Selector selector) throws IOException {
        //creates a socket for the client
        SocketChannel clientChan = serverChan.accept();
        clientChan.configureBlocking(false);
        StompMessagingProtocol<T> protocol = protocolFactory.get();
        //For every connectionHandler that the BaseServer create it give it a connectionId, and send them to Connection
        final NonBlockingConnectionHandler<T> handler = new NonBlockingConnectionHandler<>(
                readerFactory.get(),
                protocol,
                clientChan,
                this);
        idCounter++;
        connections.insertHandler(idCounter, handler);
        protocol.start(idCounter,connections);
        // configure the client socket so it can read messages.
        clientChan.register(selector, SelectionKey.OP_READ, handler);
    }

    /**
     *
     * @param key
     */
    private void handleReadWrite(SelectionKey key) {
        @SuppressWarnings("unchecked")
        NonBlockingConnectionHandler<T> handler = (NonBlockingConnectionHandler<T>) key.attachment();

        if (key.isReadable()) {
            Runnable task = handler.continueRead();
            if (task != null) {
                pool.submit(handler, task);
            }
        }

	    if (key.isValid() && key.isWritable()) {
            handler.continueWrite();
        }
    }

    /**
     *
     */
    private void runSelectionThreadTasks() {
        while (!selectorTasks.isEmpty()) {
            selectorTasks.remove().run();
        }
    }

    /**
     * Closes the connection so it won't receive/ send messages.
     * @throws IOException
     */
    @Override
    public void close() throws IOException {
        selector.close();
    }

}
