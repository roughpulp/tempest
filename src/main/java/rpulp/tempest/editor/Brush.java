package rpulp.tempest.editor;

public class Brush {

    private int x0;
    private int y0;

    public void onPressed(
            Texture texture,
            int x, int y) {
        x0 = x;
        y0 = y;
    }

    public void onReleased(
            Texture texture,
            int x, int y) {
    }

    public void onMouseDragged(
            Texture texture,
            int x, int y) {
        Line.line(x0, y0, x, y, (xx, yy) -> paint(texture, xx, yy));
        x0 = x;
        y0 = y;
    }

    private static void paint(
            Texture texture,
            int x, int y) {
        if (x >= 0 && x < texture.width() && y >= 0 && y < texture.height()) {
            texture.setArgb(x, y, 0xff000000);
        }
    }
}
