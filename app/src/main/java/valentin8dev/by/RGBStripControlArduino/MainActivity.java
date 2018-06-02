package valentin8dev.by.RGBStripControlArduino;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.AppCompatButton;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.skydoves.colorpickerview.ColorListener;
import com.skydoves.colorpickerview.ColorPickerView;

import java.text.MessageFormat;
import java.util.Objects;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    private final int REQUEST_CONNECT_DEVICE_SECURE = 1;
    private final int REQUEST_ENABLE_BT = 3;
    private BluetoothAdapter mBluetoothAdapter = null;
    private BluetoothService mBluetoothService = null;
    private float brightness = 70f;
    private SeekBar sSeekBar;
    private int[] rgbColor;
    private ColorPickerView mColorPickerView;
    private LinearLayout llColorPicker;
    private AppCompatButton btn_on, btn_off;
    private TextView tv_sb;
    public Toolbar toolbar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        /*FloatingActionButton fab = findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });*/

        mColorPickerView = findViewById(R.id.colorPickerView);
        mColorPickerView.setColorListener(new ColorListener() {
            @Override
            public void onColorSelected(int color) {
                setRGBColor(color);
            }
        });

        llColorPicker = findViewById(R.id.colorPickerLl);

        tv_sb = findViewById(R.id.seek_bar_progress);

        sSeekBar = findViewById(R.id.seekBar_rgb);
        sSeekBar.setProgress((int) getBrightness());
        sSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                sSeekBar = seekBar;
                setBrightness(progress);

                tv_sb.setText(String.valueOf(progress));
                SendMassage.setRGBLight(rgbColor, getBrightness());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                sSeekBar = seekBar;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                sSeekBar = seekBar;
            }
        });

        // Get local Bluetooth adapter
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        // If the adapter is null, then Bluetooth is not supported
        if (mBluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth is not available", Toast.LENGTH_LONG).show();
            this.finish();
        }

        btn_on = findViewById(R.id.rgb_btn_ON);
        btn_on.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SendMassage.send(2, true);
                SendMassage.setRGBLight(rgbColor, getBrightness());
                btn_on.setBackgroundColor(getResources().getColor(R.color.colorPrimary));
                btn_off.setBackgroundColor(Color.WHITE);
            }
        });

        btn_off = findViewById(R.id.rgb_btn_OFF);
        btn_off.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SendMassage.send(2, false);
                btn_off.setBackgroundColor(getResources().getColor(R.color.colorPrimary));
                btn_on.setBackgroundColor(Color.WHITE);
            }
        });

    }

    @Override
    protected void onStart() {
        super.onStart();

        // If BT is not on, request that it be enabled and do it onActivityResult
        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
            // Otherwise, setup the chat session
        } else if (mBluetoothService == null) {
            // Initialize the BluetoothService to perform bluetooth connections
            mBluetoothService = new BluetoothService(new WeakReferenceHandler(MainActivity.this));
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        // Performing this check in onResume() covers the case in which BT was
        // not enabled during onStart(), so we were paused to enable it...
        // onResume() will be called when ACTION_REQUEST_ENABLE activity returns.
        if (mBluetoothService != null) {
            // Only if the state is STATE_NONE, do we know that we haven't started already
            if (mBluetoothService.getState() == Constants.STATE_NONE) {
                // Start the Bluetooth services
                mBluetoothService.start();
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mBluetoothService != null) { // TODO: triggers when the screen rotates
            mBluetoothService.stop();
        }
    }

    private void setRGBColor(int color) {

        TextView textView = findViewById(R.id.colorPickerTv);
        String rgbColorHtml = mColorPickerView.getColorHtml();
        textView.setText(MessageFormat.format("#{0}", rgbColorHtml));

        llColorPicker.setBackgroundColor(color);

        rgbColor = mColorPickerView.getColorRGB();

        SendMassage.setRGBLight(rgbColor, getBrightness());
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case REQUEST_CONNECT_DEVICE_SECURE:
                // When DeviceListActivity returns with a device to connect
                if (resultCode == Activity.RESULT_OK) {
                    connectDevice(data);
                } else {
                    toolbar.getMenu().getItem(R.id.menu_connectToBlt)
                            .setIcon(R.drawable.ic_bluetooth_disabled_black_24dp);
                }
                break;
            case REQUEST_ENABLE_BT:
                // When the request to enable Bluetooth returns
                if (resultCode == Activity.RESULT_OK) {
                    // Bluetooth is now enabled, so set up a session
                    // Initialize the BluetoothService to perform bluetooth connections
                    mBluetoothService = new BluetoothService(new WeakReferenceHandler(MainActivity.this));
                } else {
                    // User did not enable Bluetooth or an error occurred
                    Toast.makeText(this, R.string.bt_not_enabled_leaving,
                            Toast.LENGTH_SHORT).show();
                    Objects.requireNonNull(this).finish();
                }
                break;

            // Soon™
            /*case SPEECH_RECOGNITION_CODE:
                // Callback for speech updateUI activity
                if (resultCode == MainActivity.RESULT_OK && null != data) {

                    ArrayList<String> result = data
                            .getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS);
                    String speech_recognition_text = result.get(0);
                    voiceToTxtOutput.setText(speech_recognition_text);
                    //Log.d(TAG, speech_recognition_text);

                    TextToInt.TextRecognition(speech_recognition_text);
                }
                break;*/
        }
    }

    /**
     * Establish connection with other device
     */
    private void connectDevice(Intent data) {
        // Get the device MAC address
        String address = Objects.requireNonNull(data.getExtras())
                .getString(DeviceListActivity.EXTRA_DEVICE_ADDRESS);
        // Get the BluetoothDevice object
        if (address != null) {
            BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
            // Attempt to connect to the device
            if (mBluetoothService != null) {
                mBluetoothService.connect(device);
            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();

        switch (id) {
            case R.id.menu_connectToBlt:
                if (mBluetoothService != null) {
                    if (mBluetoothService.getState() == Constants.STATE_NONE) {
                        openDeviceList();
                    } else {
                        mBluetoothService.stop();
                    }
                }
                return true;
            case R.id.menu_settings:
                // Soon™
                return true;
            case R.id.menu_about:
                openAboutActivity();
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private void openDeviceList() {
        // Launch the DeviceListActivity to see devices and do scan
        Intent serverIntent = new Intent(this, DeviceListActivity.class);
        startActivityForResult(serverIntent, REQUEST_CONNECT_DEVICE_SECURE);
    }

    private void openAboutActivity() {
        Intent aboutIntent = new Intent(this, AboutActivity.class);
        startActivityForResult(aboutIntent, RESULT_OK);
    }

    public void updateUI(){

        if (InputRecognition.rgbLight.equals("0")) {
            btn_on.setBackgroundColor(getResources().getColor(R.color.colorPrimary));
            btn_off.setBackgroundColor(Color.WHITE);
        } else if (InputRecognition.rgbLight.equals("1")) {
            btn_on.setBackgroundColor(Color.WHITE);
        }
    }

    public float getBrightness() {
        return brightness;
    }

    public void setBrightness(float brightness) {
        this.brightness = brightness;
    }
}
