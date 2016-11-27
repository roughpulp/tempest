package rpulp.tempest.editor;

import javafx.scene.paint.Color;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import rpulp.tempest.editor.blend.BlendFunctions;
import rpulp.tempest.editor.geometry.DrawLine;

import static rpulp.tempest.editor.geometry.DrawCircle.drawCircle;
import static rpulp.tempest.editor.geometry.FloodFill.floodFill;

public class Brush {

    private static final Logger LOGGER = LoggerFactory.getLogger(Brush.class);

    private Texture motif;
    private int color = 0xff0000;
    private int size = 10;

    private int x0;
    private int y0;

    public Brush() {
        updateMotif();
    }

    public Texture motif() { return motif; }

    private void closeMotif() {
        if (motif != null) {
            LOGGER.debug("closeMotif()");
            motif.close();
            motif = null;
        }
    }

    private void updateMotif() {
        LOGGER.debug("updateMotif(size: {}, color: {})", size, Integer.toHexString(color));
        {
            final int minSize = (size * 2) + 1;
            if (motif != null && motif.width() < minSize) {
                closeMotif();
            }
            if (motif == null) {
                motif = Texture.create(minSize, minSize);
            }
        }
        motif.clear(0x000000);
        int x0 = motif.width() / 2;
        int y0 = motif.height() / 2;
        drawCircle(
                x0, y0, size,
                (x, y) -> { motif.putPixel(x, y, color);});
        floodFill(motif, x0, y0, color);
    }

    public void setColor(Color color) {
        int argb = (int)(color.getRed() * 255.0) << (8 * 2)
                |  (int)(color.getGreen() * 255.0) << (8 * 1)
                |  (int)(color.getBlue() * 255.0) << (8 * 0)
                | 0xff000000;
        setColor(argb);
    }

    public void setColor(int color) {
        this.color = color;
        updateMotif();
    }

    public void setSize(int size) {
        this.size = size;
        updateMotif();
    }

    public void onPressed(Texture dst, int x, int y) {
        x0 = x;
        y0 = y;
        paintMotif(dst, x, y);
    }

    public void onReleased(Texture dst, int x, int y) {
    }

    public void onMouseDragged(Texture texture, int x, int y) {
        DrawLine.drawLine(x0, y0, x, y, (xx, yy) -> paintMotif(texture, xx, yy));
        x0 = x;
        y0 = y;
    }

    private void paintMotif(
            Texture dst,
            int x, int y) {
        int x0 = motif.width() / 2;
        int y0 = motif.height() / 2;
        dst.set(x - x0, y - y0, motif, BlendFunctions.SRC_ALPHA_ONE_MINUS_SRC_ALPHA);
    }

    private void paintPixel(
            Texture dst,
            int x, int y) {
        if ( x < 0 || x >= dst.width() || y < 0 || y >= dst.height()) {
            return;
        }
        dst.putPixel(x, y, 0xff000000);
    }
}
