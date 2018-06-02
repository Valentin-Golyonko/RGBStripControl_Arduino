package valentin8dev.by.RGBStripControlArduino;

import android.util.Log;

class InputRecognition {

    private static final String TAG = "InputRecognition";

    static String pirSensor = "--", rgbLight = "--";
    private static int l = -1, w = -1, p = -1, u = -1;

    private static boolean date_came = false;

    /**
     * resolve input string 'in' to data (int) and parse it to the Activity (view elements)
     * <p>
     * ex: input = p0ul0w
     * pir = 0
     * rgbLight = 0
     */
    static void inputSymbols(String input) {

        if (input != null) {
            //Log.d(TAG, "input " + input);

            setDate_came(true);

            int input_length = input.length();

            for (int i = 0; i < input_length; i++) {
                switch (input.charAt(i)) {
                    case 'l': l = i + 1; break;
                    case 'w': w = i; break;
                    case 'p': p = i + 1; break;
                    case 'u': u = i; break;
                }
            }

            rgbLight = parameter(input, l, w);
            l = w = -1;

            pirSensor = parameter(input, p, u);
            p = u = -1;
        }

        setDate_came(false);
    }

    private static String parameter(String in, int from, int to) {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(in);

        if (from != -1 && to != -1) {
            int stringSize = to - from;
            //Log.d(TAG, "from " + from + " to " + to + " s " + stringSize);
            if (stringSize > 0) { // protection from outOfBoundEx-n
                //Log.d(TAG, "param " + param);

                return stringBuilder.substring(from, to);
            }
        }
        return "-1";
    }

    static void setDate_came(boolean date_came) {
        InputRecognition.date_came = date_came;
        Log.d(TAG, "date_came " + date_came);
    }

    static boolean isDate_came() {
        return date_came;
    }
}
