package valentin8dev.by.RGBStripControlArduino;

import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

import java.lang.ref.WeakReference;

public class WeakReferenceHandler extends Handler {

    private static final String TAG = WeakReferenceHandler.class.getSimpleName();

    private final WeakReference<MainActivity> bluetoothFragmentWeakReference;
    private StringBuilder sb = new StringBuilder();

    WeakReferenceHandler(MainActivity mainActivity) {
        bluetoothFragmentWeakReference = new WeakReference<>(mainActivity);
    }

    @Override
    public void handleMessage(Message msg) {
        super.handleMessage(msg);

        MainActivity mainActivity = bluetoothFragmentWeakReference.get();

        if (mainActivity != null) {

            switch (msg.what) {
                case Constants.MESSAGE_STATE_CHANGE:
                    switch (msg.arg1) {
                        case Constants.STATE_CONNECTED:
                            //
                            break;
                        case Constants.STATE_CONNECTING:
                            //
                            break;
                        case Constants.STATE_LISTEN:
                            break;
                        case Constants.STATE_NONE:
                            //
                            break;
                    }
                    break;
                case Constants.MESSAGE_READ:
                    byte[] readBuf = (byte[]) msg.obj;  // construct a string from the valid bytes in the buffer
                    String readMessage = new String(readBuf, 0, msg.arg1);
                    sb.append(readMessage);

                    //Log.d(TAG, "sb " + sb.toString() + " " + sb.length());
                    /* example: sb p0ul0wE 9 */

                    int endOfLineIndex = sb.indexOf("E");   // end of the receiving line index
                    if (endOfLineIndex > 0) {
                        String sbPrint = sb.substring(0, endOfLineIndex);
                        sb.delete(0, sb.length());
                        sb.trimToSize();

                        InputRecognition.inputSymbols(sbPrint); // resolve input string to data (int)
                        mainActivity.updateUI();                             // and than update layout
                    }
                    if (sb.length() > 64) {
                        sb.delete(0, sb.length());
                        Log.w(TAG, "sb too large");
                    }
                    break;
                case Constants.MESSAGE_DEVICE_NAME:
                    // save the connected device's name
                    String connectedDeviceName = msg.getData().getString(Constants.DEVICE_NAME);

                    Toast.makeText(mainActivity, "Connected to "
                            + connectedDeviceName, Toast.LENGTH_SHORT).show();

                    mainActivity.toolbar.getMenu().getItem(0)
                            .setIcon(R.drawable.ic_bluetooth_audio_black_24dp);

                    break;
                case Constants.MESSAGE_TOAST:
                    Toast.makeText(mainActivity, msg.getData().getString(Constants.TOAST),
                            Toast.LENGTH_SHORT).show();

                    // check connection state and switcher
                    if (BluetoothService.state == 0 || BluetoothService.state == 1) {
                        mainActivity.toolbar.getMenu().getItem(0)
                                .setIcon(R.drawable.ic_bluetooth_disabled_black_24dp);
                    }
                    break;
            }
        }
    }
}
