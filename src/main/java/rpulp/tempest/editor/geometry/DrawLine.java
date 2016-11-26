package rpulp.tempest.editor.geometry;

public class DrawLine {
    public static void drawLine(
            int x0, int y0,
            int x1, int y1,
            Action action) {
        int dx = x1 - x0;
        if (dx == 0) {
            int dy = y1 - y0;
            int delta = dy >= 0 ? 1 : -1;
            for (int yy = y0; yy != y1; yy += delta) {
                action.at(x0, yy);
            }
            return;
        }
        int dy = y1 - y0;
        if (Math.abs(dx) >= Math.abs(dy)) {
            int inc = dx >= 0 ? 1 : -1;
            double dd = (double) dy / (double) dx;
            for (int xx = x0; xx != x1; xx += inc) {
                int yy = y0 + (int) (dd * (xx - x0));
                action.at(xx, yy);
            }
        } else {
            int inc = dy >= 0 ? 1 : -1;
            double dd = (double) dx / (double) dy;
            for (int yy = y0; yy != y1; yy += inc) {
                int xx = x0 + (int) (dd * (yy - y0));
                action.at(xx, yy);
            }
        }
    }
}
