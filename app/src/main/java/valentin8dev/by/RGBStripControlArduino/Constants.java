package valentin8dev.by.RGBStripControlArduino;

/**
 * Defines constants
 */
public interface Constants {

    // Message types sent from the BluetoothService Handler
    int MESSAGE_STATE_CHANGE = 1;
    int MESSAGE_READ = 2;
    int MESSAGE_WRITE = 3;
    int MESSAGE_DEVICE_NAME = 4;
    int MESSAGE_TOAST = 5;

    // Key names received from the BluetoothService Handler
    String DEVICE_NAME = "device_name";
    String TOAST = "toast";

    String turnOnLight_ru = "Включить свет";       // send 3002 (pin 3, high),  code '2', coz i torning On RGBStrip forcibly!
    String turnOffLight_ru = "Выключить свет";       // send 3000 (pin 3, low)
    String turnOnLight_eng = "Turn on the Lights";       // send 3002 (pin 3, high),  code '2', coz i torning On RGBStrip forcibly!
    String turnOffLight_eng = "Turn Off the Lights";       // send 3000 (pin 3, low)

    // Constants that indicate the current connection state
    int STATE_NONE = 0;       // we're doing nothing
    int STATE_LISTEN = 1;     // now listening for incoming connections
    int STATE_CONNECTING = 2; // now initiating an outgoing connection
    int STATE_CONNECTED = 3;  // now connected to a remote device
}