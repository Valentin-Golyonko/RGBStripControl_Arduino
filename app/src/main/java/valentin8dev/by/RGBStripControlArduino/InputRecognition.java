package valentin8dev.by.RGBStripControlArduino;

class InputRecognition {

    private static final String TAG = InputRecognition.class.getSimpleName();

    static String pirSensor = "--", rgbLight = "--", photoRes = "--", autoB = "--";
    private static int l = -1, w = -1, p = -1, u = -1, f = -1, r = -1, a = -1, b = -1;

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

            int input_length = input.length();

            for (int i = 0; i < input_length; i++) {
                switch (input.charAt(i)) {
                    case 'l': l = i + 1; break;
                    case 'w': w = i; break;
                    case 'p': p = i + 1; break;
                    case 'u': u = i; break;
                    case 'f': f = i + 1; break;
                    case 'r': r = i; break;
                    case 'a': a = i + 1; break;
                    case 'b': b = i; break;
                }
            }

            rgbLight = parameter(input, l, w);
            l = w = -1;

            pirSensor = parameter(input, p, u);
            p = u = -1;

            photoRes = parameter(input, f, r);
            f = r = -1;

            autoB = parameter(input, a, b);
            a = b = -1;
        }
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
}
