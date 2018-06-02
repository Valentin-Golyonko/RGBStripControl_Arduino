package valentin8dev.by.RGBStripControlArduino;

import android.util.Log;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

class SendMassage {

    private static final String TAG = "SendMassage";
    /**
     * all SendMassage in one 'switch' :
     * 1 = bluetooth status
     * 2 = rgb strip switcher status
     */

    private static int updatePeriod = 3;    // period of sending a message from the station = 3 sec

    static void send(int i, boolean bool) {
        try {
            //Get the output stream for data transfer
            if (BluetoothService.getBltSocket() != null) {
                OutputStream outputStream = BluetoothService.getBltSocket().getOutputStream();
                int value = -1;

                switch (i) {
                    case 1: // bltStatus
                        // Depending on which button was pressed, we change the data to send
                        if (bool) {
                            value = 1000 + updatePeriod;    // BLT ON, ready to receive data, period = 5sec
                        } else {
                            value = 2000;    // BLT OFF, stop transmit from arduino
                            InputRecognition.setDate_came(false);
                        }
                        break;
                    case 2: // switchRgbLight
                        if (bool) {
                            value = 3002;    // RGBStrip ON always
                        } else {
                            value = 3000;
                        }
                        break;
                }

                //Log.d(TAG, String.valueOf(value));

                // Write the data to the output stream
                byte[] bytes = ByteBuffer.allocate(4).putInt(value).array();
                //Log.d(TAG, Arrays.toString(bytes));

                for (byte b : bytes) {
                    outputStream.write(b);
                }

            }
        } catch (IOException e) {
            Log.e(TAG, "send outputStream", e);
        }
    }

    static void setRGBLight(int[] color, float progress) {

        // R = 10 255 - 10 000  color[0]
        // G = 11 255 - 11 000  color[1]
        // B = 13 255 - 13 000  color[2],
        // 13 = prefix code for blue pin 9, coz 90255 does't get in to 2 bytes :(

        if (BluetoothService.getBltSocket() != null) {
            try {
                OutputStream outputStream = BluetoothService.getBltSocket().getOutputStream();

                int[] value = new int[3];
                value[0] = 10000 + (int) (color[0] * progress / 100);
                value[1] = 11000 + (int) (color[1] * progress / 100);
                value[2] = 13000 + (int) (color[2] * progress / 100);

                //Log.d(TAG, "RGB: " + Arrays.toString(value));

                // set rgb strip color
                for (int i = 0; i < 3; i++) {
                    byte[] bytes_2 = ByteBuffer.allocate(4).putInt(value[i]).array();
                    //Log.d(TAG, Arrays.toString(bytes_2));

                    for (byte b : bytes_2) {
                        outputStream.write(b);
                    }
                }
            } catch (IOException e) {
                Log.e(TAG, "setRGBLight outputStream", e);
            }
        }
    }

    static void setUpdatePeriod(int updatePeriod) {
        SendMassage.updatePeriod = updatePeriod;
    }
}
