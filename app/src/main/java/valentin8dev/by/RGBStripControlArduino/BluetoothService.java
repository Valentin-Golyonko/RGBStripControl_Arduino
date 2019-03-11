package valentin8dev.by.RGBStripControlArduino;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.UUID;

/**
 * This class does all the work for setting up and managing Bluetooth
 * connections with other devices. It has a thread that listens for
 * incoming connections, a thread for connecting with a device, and a
 * thread for performing data transmissions when connected.
 */
class BluetoothService {

    private static final String TAG = BluetoothService.class.getSimpleName();

    // Unique UUID for this application
    private final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    static int state;

    private static BluetoothSocket bltSocket;
    private final BluetoothAdapter bltAdapter;
    private final Handler mHandler;
    private AcceptThread secureAcceptThread;
    private ConnectThread connectThread;
    private ConnectedThread connectedThread;
    private int newState;

    /**
     * Constructor. Prepares a new BluetoothChat session.
     * handler send messages back to the UI Activity
     */
    BluetoothService(Handler handler) {
        bltAdapter = BluetoothAdapter.getDefaultAdapter();
        state = Constants.STATE_NONE;
        newState = state;
        mHandler = handler;
    }

    static BluetoothSocket getBltSocket() {
        return bltSocket;
    }

    // Update UI title according to the current state of the chat connection
    private synchronized void updateState() {
        state = getState();
        //Log.d(TAG, "updateState() " + newState + " -> " + state);
        newState = state;

        // Give the new state to the Handler so the UI Activity can update
        mHandler.obtainMessage(Constants.MESSAGE_STATE_CHANGE, newState, -1).sendToTarget();
    }

    /**
     * Return the current connection state.
     */
    synchronized int getState() {
        return state;
    }

    /**
     * Start the chat service. Specifically start AcceptThread to begin a
     * session in listening (server) mode. Called by the Activity onResume()
     */
    synchronized void start() {
        //Log.d(TAG, "start");

        // Cancel any thread attempting to make a connection
        if (connectThread != null) {
            connectThread.cancel();
            connectThread = null;
        }

        // Cancel any thread currently running a connection
        if (connectedThread != null) {
            connectedThread.cancel();
            connectedThread = null;
        }

        // Start the thread to listen on a BluetoothServerSocket
        if (secureAcceptThread == null) {
            secureAcceptThread = new AcceptThread();
            secureAcceptThread.start();
        }
        // Update UI title
        updateState();
    }

    /**
     * Start the ConnectThread to initiate a connection to a remote device.
     */
    synchronized void connect(BluetoothDevice device) {
        //Log.d(TAG, "connect to: " + device);

        // Cancel any thread attempting to make a connection
        if (state == Constants.STATE_CONNECTING) {
            if (connectThread != null) {
                connectThread.cancel();
                connectThread = null;
            }
        }

        // Cancel any thread currently running a connection
        if (connectedThread != null) {
            connectedThread.cancel();
            connectedThread = null;
        }

        // Start the thread to connect with the given device
        connectThread = new ConnectThread(device);
        connectThread.start();
        // Update UI title
        updateState();
    }

    /**
     * Start the ConnectedThread to begin managing a Bluetooth connection
     */
    private synchronized void connected(BluetoothSocket socket, BluetoothDevice device) {
        //Log.d(TAG, "connected");

        // Cancel the thread that completed the connection
        if (connectThread != null) {
            connectThread.cancel();
            connectThread = null;
        }

        // Cancel any thread currently running a connection
        if (connectedThread != null) {
            connectedThread.cancel();
            connectedThread = null;
        }

        // Cancel the accept thread because we only want to connect to one device
        if (secureAcceptThread != null) {
            secureAcceptThread.cancel();
            secureAcceptThread = null;
        }

        // Start the thread to manage the connection and perform transmissions
        connectedThread = new ConnectedThread(socket);
        connectedThread.start();

        // Send the name of the connected device back to the UI Activity
        Message msg = mHandler.obtainMessage(Constants.MESSAGE_DEVICE_NAME);
        Bundle bundle = new Bundle();
        bundle.putString(Constants.DEVICE_NAME, device.getName());
        msg.setData(bundle);
        mHandler.sendMessage(msg);
        // Update UI title
        updateState();
    }

    /**
     * Stop all threads
     */
    synchronized void stop() {
        //Log.d(TAG, "stop");

        write();

        if (connectThread != null) {
            connectThread.cancel();
            connectThread = null;
        }

        if (connectedThread != null) {
            connectedThread.cancel();
            connectedThread = null;
        }

        if (secureAcceptThread != null) {
            secureAcceptThread.cancel();
            secureAcceptThread = null;
        }

        state = Constants.STATE_NONE;
        // Update UI title
        updateState();
    }

    /**
     * Write to the ConnectedThread in an unsynchronized manner
     */
    private void write() {
        // Create temporary object
        ConnectedThread r;
        // Synchronize a copy of the ConnectedThread
        synchronized (this) {
            if (state != Constants.STATE_CONNECTED) return;
            r = connectedThread;
        }
        // Perform the write unsynchronized
        r.write(ByteBuffer.allocate(4).putInt(2000).array());
    }

    /**
     * Indicate that the connection attempt failed and notify the UI Activity.
     */
    private void connectionFailed() {
        // Send a failure message back to the Activity
        Message msg = mHandler.obtainMessage(Constants.MESSAGE_TOAST);
        Bundle bundle = new Bundle();
        bundle.putString(Constants.TOAST, "Unable to connect device");
        msg.setData(bundle);
        mHandler.sendMessage(msg);

        state = Constants.STATE_NONE;
        // Update UI title
        updateState();

        // Start the service over to restart listening mode
        BluetoothService.this.start();
    }

    /**
     * Indicate that the connection was lost and notify the UI Activity.
     */
    private void connectionLost() {
        // Send a failure message back to the Activity
        Message msg = mHandler.obtainMessage(Constants.MESSAGE_TOAST);
        Bundle bundle = new Bundle();
        bundle.putString(Constants.TOAST, "Device connection was lost");
        msg.setData(bundle);
        mHandler.sendMessage(msg);

        state = Constants.STATE_NONE;
        // Update UI title
        updateState();

        // Start the service over to restart listening mode
        BluetoothService.this.start();
    }

    /**
     * This thread runs while listening for incoming connections. It behaves
     * like a server-side client. It runs until a connection is accepted
     * (or until cancelled).
     */
    private class AcceptThread extends Thread {
        // The local server socket
        private final BluetoothServerSocket mmServerSocket;

        AcceptThread() {
            BluetoothServerSocket tmp = null;

            // Create a new listening server socket
            try {
                tmp = bltAdapter.listenUsingRfcommWithServiceRecord("BluetoothSecure",
                        MY_UUID);

            } catch (IOException e) {
                //Log.e(TAG, "listen() failed", e);
            }
            mmServerSocket = tmp;
            state = Constants.STATE_LISTEN;

            //Log.d(TAG, "AcceptThread");
        }

        public void run() {

            BluetoothSocket socket;

            // Listen to the server socket if we're not connected
            while (state != Constants.STATE_CONNECTED) {
                try {
                    // This is a blocking call and will only return on a
                    // successful connection or an exception
                    socket = mmServerSocket.accept();
                } catch (IOException e) {
                    Log.e(TAG, "accept() failed", e);
                    break;
                }

                // If a connection was accepted
                if (socket != null) {
                    Log.e(TAG, "socket != null");
                    synchronized (BluetoothService.this) {
                        switch (state) {
                            case Constants.STATE_LISTEN:
                            case Constants.STATE_CONNECTING:
                                // Situation normal. Start the connected thread.
                                connected(socket, socket.getRemoteDevice());
                                break;
                            case Constants.STATE_NONE:
                            case Constants.STATE_CONNECTED:
                                // Either not ready or already connected. Terminate new socket.
                                try {
                                    socket.close();
                                } catch (IOException e) {
                                    Log.e(TAG, "Could not close unwanted socket", e);
                                }
                                break;
                        }
                    }
                }
            }
            //Log.i(TAG, "END mAcceptThread");
        }

        void cancel() {
            //Log.d(TAG, "cancel " + this);
            try {
                mmServerSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "close() of server failed", e);
            }
        }
    }

    /**
     * This thread runs while attempting to make an outgoing connection
     * with a device. It runs straight through; the connection either
     * succeeds or fails.
     */
    private class ConnectThread extends Thread {
        private final BluetoothDevice mmDevice;

        ConnectThread(BluetoothDevice device) {
            mmDevice = device;
            BluetoothSocket tmp = null;

            // Get a BluetoothSocket for a connection with the
            // given BluetoothDevice
            try {
                tmp = device.createRfcommSocketToServiceRecord(
                        MY_UUID);
            } catch (IOException e) {
                Log.e(TAG, "create() failed", e);
            }
            bltSocket = tmp;
            state = Constants.STATE_CONNECTING;
        }

        public void run() {

            // Always cancel discovery because it will slow down a connection
            bltAdapter.cancelDiscovery();

            // Make a connection to the BluetoothSocket
            try {
                // This is a blocking call and will only return on a
                // successful connection or an exception
                bltSocket.connect();
            } catch (IOException e) {
                // Close the socket
                try {
                    bltSocket.close();
                } catch (IOException e2) {
                    Log.e(TAG, "unable to close() socket during connection failure", e2);
                }
                connectionFailed();
                return;
            }

            // Reset the ConnectThread because we're done
            synchronized (BluetoothService.this) {
                connectThread = null;
            }

            // Start the connected thread
            connected(bltSocket, mmDevice);
        }

        void cancel() {
            try {
                bltSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "close() socket failed", e);
            }
        }
    }

    /**
     * This thread runs during a connection with a remote device.
     * It handles all incoming and outgoing transmissions.
     */
    private class ConnectedThread extends Thread {

        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        ConnectedThread(BluetoothSocket socket) {
            //Log.d(TAG, "create ConnectedThread");
            bltSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the BluetoothSocket input and output streams
            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
                Log.e(TAG, "temp sockets not created", e);
            }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
            state = Constants.STATE_CONNECTED;
        }

        public void run() {
            //Log.d(TAG, "BEGIN connectedThread");
            byte[] buffer = new byte[64];
            int bytes;

            // send flag that tells arduino: android app is ready to receive date!
            SendMassage.send(1, true, 0);

            // Keep listening to the InputStream while connected
            while (state == Constants.STATE_CONNECTED) {
                try {
                    // Read from the InputStream
                    bytes = mmInStream.read(buffer);

                    //Log.d(TAG, "-----");
                    //Log.d(TAG, "buffer[] " + Arrays.toString(buffer));
                    //Log.d(TAG, "bytes " + bytes);

                    // Send the obtained bytes to the UI Activity
                    mHandler.obtainMessage(Constants.MESSAGE_READ, bytes, -1, buffer)
                            .sendToTarget();
                } catch (IOException e) {
                    Log.e(TAG, "disconnected", e);
                    connectionLost();
                    break;
                }
            }
        }

        /**
         * Write to the connected OutStream.
         */
        void write(byte[] buffer) {
            try {
                mmOutStream.write(buffer);

                // Share the sent message back to the UI Activity
                mHandler.obtainMessage(Constants.MESSAGE_WRITE, -1, -1, buffer)
                        .sendToTarget();
            } catch (IOException e) {
                Log.e(TAG, "Exception during write", e);
            }
        }

        void cancel() {
            try {
                bltSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "close() of connect socket failed", e);
            }
        }
    }
}