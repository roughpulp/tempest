package rpulp.tempest.editor;

public class Clock {

    private static final long tstamp0 = System.nanoTime();

    public static long tstamp() {
        return System.nanoTime() - tstamp0;
    }

}
