package valentin8dev.by.RGBStripControlArduino;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

class TextToInt {

    private static final String TAG = TextToInt.class.getSimpleName();

    static void TextRecognition(String string) {

        if (string != null) {
            if (BluetoothService.getBltSocket() != null) {
                try {
                    OutputStream outputStream = BluetoothService.getBltSocket().getOutputStream();
                    int value = 0;

                    switch (string) {
                        case Constants.turnOnLight_ru:
                            //Log.d(TAG, Constants.turnOnLight_ru);
                            value = 3002;   // RGBStrip ON always
                            break;
                        case Constants.turnOffLight_ru:
                            //Log.d(TAG, Constants.turnOffLight_ru);
                            value = 3000;
                            break;
                        case Constants.turnOnLight_eng:
                            //Log.d(TAG, Constants.turnOnLight_eng);
                            value = 3002;   // RGBStrip ON always
                            break;
                        case Constants.turnOffLight_eng:
                            //Log.d(TAG, Constants.turnOffLight_eng);
                            value = 3000;
                            break;
                    }

                    byte[] bytes = ByteBuffer.allocate(4).putInt(value).array();
                    //Log.d(TAG, Arrays.toString(bytes));

                    for (byte b : bytes) {
                        outputStream.write(b);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}