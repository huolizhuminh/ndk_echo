package com.minhuizhu.echo;

/**
 * Created by zhuminh on 2017/1/2.
 */

public class EchoServerActivity extends AbstractEchoActivity {
    public EchoServerActivity() {
        super(R.layout.activity_echo_server);
    }

    @Override
    protected void onStartButtonClicked() {
        Integer port = getPort();
        if (port != null) {
            ServerTask serverTask = new ServerTask(port);
            serverTask.start();
        }
    }
    private class ServerTask extends AbstractEchoTask{
        /** Port number. */
        private final int port;

        /**
         * Constructor.
         *
         * @param port
         *            port number.
         */
        public ServerTask(int port) {
            this.port = port;
        }

        protected void onBackground() {
            logMessage("Starting server.");

            try {
              nativeStartTcpServer(port);
                nativeStartUdpServer(port);
            } catch (Exception e) {
                logMessage(e.getMessage());
            }

            logMessage("Server terminated.");
        }
    }
    private native void nativeStartTcpServer(int port) ;
    private native void nativeStartUdpServer(int port);
}

