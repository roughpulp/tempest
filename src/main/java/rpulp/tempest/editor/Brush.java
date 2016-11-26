package rpulp.tempest.editor;

import rpulp.tempest.editor.blend.BlendFunctions;
import rpulp.tempest.editor.geometry.DrawLine;

import static rpulp.tempest.editor.geometry.DrawCircle.drawCircle;
import static rpulp.tempest.editor.geometry.FloodFill.floodFill;

public class Brush {

    private Texture motif;
    private int color = 0xffff0000;
    private int size = 10;

    private int x0;
    private int y0;

    public Brush() {
        int width = (size * 2) + 1;
        int height = (size * 2) + 1;
        motif = Texture.create(width, height);
        motif.fill(0);
        drawCircle(
                size, size,
                size,
                (x, y) -> motif.putPixel(x, y, color));
        floodFill(motif, size, size, color);
    }

    public void onPressed(Texture texture, int x, int y) {
        x0 = x;
        y0 = y;
        paintMotif(texture, x, y);
    }

    public void onReleased(Texture texture, int x, int y) {
    }

    public void onMouseDragged(Texture texture, int x, int y) {
        DrawLine.drawLine(x0, y0, x, y, (xx, yy) -> paintMotif(texture, xx, yy));
        x0 = x;
        y0 = y;
    }

    private static void paint(
            Texture texture,
            int x, int y) {
        if ( x < 0 || x >= texture.width() || y < 0 || y >= texture.height()) {
            return;
        }
        texture.putPixel(x, y, 0xff000000);
    }

    private void paintMotif(
            Texture texture,
            int x, int y) {
        texture.set(x - size, y - size, motif, BlendFunctions.SRC_ALPHA_ONE_MINUS_SRC_ALPHA);
    }
}
