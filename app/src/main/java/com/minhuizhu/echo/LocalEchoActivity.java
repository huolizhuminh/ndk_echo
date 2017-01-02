package com.minhuizhu.echo;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.widget.EditText;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by zhuminh on 2017/1/2.
 */

public class LocalEchoActivity extends AbstractEchoActivity {
    /**
     * Message edit.
     */
    private EditText messageEdit;

    public LocalEchoActivity() {
        super(R.layout.activity_local_echo);
    }

    @Override
    public void onCreate(Bundle savedInstanceState, PersistableBundle persistentState) {
        super.onCreate(savedInstanceState, persistentState);
        messageEdit = (EditText) findViewById(R.id.message_edit);
    }

    @Override
    protected void onStartButtonClicked() {
        String name = portEdit.getText().toString();
        String message = messageEdit.getText().toString();

        if ((name.length() > 0) && (message.length() > 0)) {
            String socketName;

            // If it is a filesystem socket, prepend the
            // application files directory
            if (isFilesystemSocket(name)) {
                File file = new File(getFilesDir(), name);
                socketName = file.getAbsolutePath();
            } else {
                socketName = name;
            }

            ServerTask serverTask = new ServerTask(socketName);
            serverTask.start();

            ClientTask clientTask = new ClientTask(socketName, message);
            clientTask.start();
        }
    }

    private boolean isFilesystemSocket(String name) {
        return name.startsWith("/");
    }

    private class ServerTask extends AbstractEchoTask {
        /**
         * Socket name.
         */
        private final String name;

        /**
         * Constructor.
         *
         * @param name socket name.
         */
        public ServerTask(String name) {
            this.name = name;
        }

        protected void onBackground() {
            logMessage("Starting server.");

            try {
                nativeStartLocalServer(name);
            } catch (Exception e) {
                logMessage(e.getMessage());
            }

            logMessage("Server terminated.");
        }
    }

    /**
     * Client task.
     */
    private class ClientTask extends Thread {
        /**
         * Socket name.
         */
        private final String name;

        /**
         * Message text to send.
         */
        private final String message;

        /**
         * Constructor.
         *
         * @param message message text to send.
         * @parma name socket name.
         */
        public ClientTask(String name, String message) {
            this.name = name;
            this.message = message;
        }

        public void run() {
            logMessage("Starting client.");

            try {
                startLocalClient(name, message);
            } catch (Exception e) {
                logMessage(e.getMessage());
            }

            logMessage("Client terminated.");
        }
    }

    private void startLocalClient(String name, String message) {
        LocalSocket clientSocket = new LocalSocket();
        try {
            LocalSocketAddress.Namespace nameSpace;
            if (isFilesystemSocket(name)) {
                nameSpace = LocalSocketAddress.Namespace.FILESYSTEM;
            } else {
                nameSpace = LocalSocketAddress.Namespace.ABSTRACT;
            }
            LocalSocketAddress address = new LocalSocketAddress(name, nameSpace);
            logMessage("Connecting ot"+name);
            clientSocket.connect(address);
            logMessage("Connected");
            byte[] bytes = message.getBytes();
            logMessage("Sending to the socket");
            OutputStream outputStream = clientSocket.getOutputStream();
            outputStream.write(bytes);
            logMessage(String.format("Sent %d bytes: %s",bytes.length,message));
            InputStream inputStream = clientSocket.getInputStream();
            int readSize = inputStream.read(bytes);
            String receivedMessage = new String(bytes, 0, readSize);
            outputStream.close();
            inputStream.close();
        } catch (Throwable e) {
            logMessage(e.getMessage());
        } finally {

        }
    }


    private native void nativeStartLocalServer(String name);

}














































