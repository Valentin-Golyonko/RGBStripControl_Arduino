package valentin8dev.by.RGBStripControlArduino;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.ToggleButton;

import java.util.Calendar;
import java.util.Objects;

public class SettingsActivity extends Activity {

    private static final String TAG = SettingsActivity.class.getSimpleName();

    private EditText et_time_h, et_time_m;
    private ToggleButton tb_mon, tb_tue, tb_wed, tb_thu, tb_fri, tb_sat, tb_sun, tb_auto_b;
    private TextView tv_auto_b;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        et_time_h = findViewById(R.id.et_time_h);
        et_time_m = findViewById(R.id.et_time_m);
        tb_mon = findViewById(R.id.tb1);
        tb_tue = findViewById(R.id.tb2);
        tb_wed = findViewById(R.id.tb3);
        tb_thu = findViewById(R.id.tb4);
        tb_fri = findViewById(R.id.tb5);
        tb_sat = findViewById(R.id.tb6);
        tb_sun = findViewById(R.id.tb7);
        tb_auto_b = findViewById(R.id.tb_auto_b);
        tv_auto_b = findViewById(R.id.tv_auto_b);

        getTodayDay();

        Button btn_set_alarm = findViewById(R.id.btn_set_alarm);
        btn_set_alarm.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                if (getAndSetTime() != -1) {
                    boolean[] arr = getAndSetDay();
                    for (int i = 0; i < 7; i++) {
                        if (arr[i]) {

                            SendMassage.send(4, true, (i + 1));
                            SendMassage.send(40, true, getAndSetTime());
                            Log.d(TAG, "day " + i + " time " + getAndSetTime());
                        }
                    }
                }
            }
        });

        Button btn_alarm_off = findViewById(R.id.btn_alarm_off);
        btn_alarm_off.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                boolean[] arr = getAndSetDay();
                for (int i = 0; i < 7; i++) {
                    if (arr[i]) {
                        SendMassage.send(6, true, i);
                        Log.d(TAG, "alarm off " + i);
                    }
                }
            }
        });

        if (Objects.equals(InputRecognition.autoB, "1")) {
            tb_auto_b.setChecked(true);
        } else {
            tb_auto_b.setChecked(false);
        }
        tb_auto_b.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (tb_auto_b.isChecked()) {
                    SendMassage.send(5, false, 0);
                } else if (!tb_auto_b.isChecked()) {
                    SendMassage.send(5, true, 0);
                }
            }
        });

        tv_auto_b.setText(InputRecognition.photoRes);
    }

    @NonNull
    @org.jetbrains.annotations.Contract(" -> new")
    private boolean[] getAndSetDay() {
        return new boolean[]{
                tb_sun.isChecked(),
                tb_mon.isChecked(),
                tb_tue.isChecked(),
                tb_wed.isChecked(),
                tb_thu.isChecked(),
                tb_fri.isChecked(),
                tb_sat.isChecked()};
    }

    private int getAndSetTime() {
        String alarm_hh = et_time_h.getText().toString();
        String alarm_mm = et_time_m.getText().toString();
        String sb = alarm_hh + alarm_mm;

        if (!alarm_hh.isEmpty() & !alarm_mm.isEmpty()) {
            return Integer.parseInt(sb);
        }
        return -1;
    }

    private void getTodayDay() {
        Calendar calendar = Calendar.getInstance();
        int d = calendar.get(Calendar.DAY_OF_WEEK);
        switch (d) {
            case 1: tb_sun.setChecked(true); break;
            case 2: tb_mon.setChecked(true); break;
            case 3: tb_tue.setChecked(true); break;
            case 4: tb_wed.setChecked(true); break;
            case 5: tb_thu.setChecked(true); break;
            case 6: tb_fri.setChecked(true); break;
            case 7: tb_sat.setChecked(true); break;
        }
    }
}
