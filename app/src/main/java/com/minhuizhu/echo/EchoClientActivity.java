package com.minhuizhu.echo;

import android.os.Bundle;
import android.os.PersistableBundle;
import android.widget.EditText;

/**
 * Created by zhuminh on 2017/1/2.
 */

public class EchoClientActivity extends AbstractEchoActivity {
    /** IP address. */
    private EditText ipEdit;

    /** Message edit. */
    private EditText messageEdit;

    /**
     * Constructor.
     */
    public EchoClientActivity() {
        super(R.layout.activity_echo_client);
    }

    @Override
    public void onCreate(Bundle savedInstanceState, PersistableBundle persistentState) {
        super.onCreate(savedInstanceState, persistentState);
        ipEdit = (EditText) findViewById(R.id.ip_edit);
        messageEdit = (EditText) findViewById(R.id.message_edit);
    }

    @Override
    protected void onStartButtonClicked() {
        String ip = ipEdit.getText().toString();
        Integer port = getPort();
        String message = messageEdit.getText().toString();

        if ((0 != ip.length()) && (port != null) && (0 != message.length())) {
            ClientTask clientTask = new ClientTask(ip, port, message);
            clientTask.start();
        }
    }
    private class ClientTask extends AbstractEchoTask{
        /** IP address to connect. */
        private final String ip;

        /** Port number. */
        private final int port;

        /** Message text to send. */
        private final String message;

        /**
         * Constructor.
         *
         * @param ip
         *            IP address to connect.
         * @param port
         *            port number to connect.
         * @param message
         *            message text to send.
         */
        public ClientTask(String ip, int port, String message) {
            this.ip = ip;
            this.port = port;
            this.message = message;
        }

        @Override
        protected void onBackground() {
            logMessage("Starting client.");
            try {
                nativeStartUdpClient(ip,port,message);
            }catch (Throwable e){
                logMessage(e.getMessage());
            }
            logMessage("Client terminated.");
        }
    }
    private native void nativeStartUdpClient(String ip, int port, String message) ;
}













































