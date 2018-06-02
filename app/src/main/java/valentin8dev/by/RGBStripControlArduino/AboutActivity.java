package valentin8dev.by.RGBStripControlArduino;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.text.util.Linkify;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public class AboutActivity extends Activity {

    private static final String TAG = "AboutActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.about_menu);

        TextView tv_hello = findViewById(R.id.tv_menu_hello);
        Linkify.addLinks(tv_hello, Linkify.WEB_URLS);

        // Set result CANCELED in case the user backs out
        setResult(Activity.RESULT_CANCELED);
    }
}
