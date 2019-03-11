package valentin8dev.by.RGBStripControlArduino;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import java.util.Arrays;

public class SettingsActivity extends Activity {

    private static final String TAG = SettingsActivity.class.getSimpleName();

    EditText et_time;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        et_time = findViewById(R.id.et_time);

        Button btn_ok = findViewById(R.id.btn_settings_ok);
        btn_ok.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String etStr = et_time.getText().toString();
                char[] c = etStr.toCharArray();

                String hhmm = String.valueOf(c[0]) + c[1] + c[3] + c[4];    // TODO: input check
                int alarm_hhmm = Integer.parseInt(hhmm);

                //SendMassage.send(5, true, 0);
                SendMassage.send(4, true, 0);
                SendMassage.send(40, true, alarm_hhmm);

                Log.d(TAG, alarm_hhmm + " SettingsActivity: btn_ok");
                setResult(Activity.RESULT_OK);
                finish();
            }
        });

        // Set result CANCELED in case the user backs out
        setResult(Activity.RESULT_CANCELED);
    }
}
