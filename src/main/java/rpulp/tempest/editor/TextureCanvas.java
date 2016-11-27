package rpulp.tempest.editor;

import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.image.PixelWriter;
import javafx.scene.paint.Color;

import java.nio.IntBuffer;

public class TextureCanvas {

    public interface TextureProvider {
        Texture texture();
    }

    private final Canvas canvas;
    private final TextureProvider textureProvider;

    public TextureCanvas(TextureProvider textureProvider, int width, int height) {
        this.textureProvider = textureProvider;
        this.canvas = new Canvas(width, height);
    }

    public Canvas canvas() { return canvas; }

    public void updateCanvas() {
        GraphicsContext gfx = canvas.getGraphicsContext2D();
        gfx.setFill(Color.WHITE);
        gfx.fillRect(0, 0, canvas.getWidth(), canvas.getHeight());
        Texture texture = textureProvider.texture();
        int x0 = Math.max(0, (int) ((canvas.getWidth() - texture.width()) / 2.0));
        int y0 = Math.max(0, (int) ((canvas.getHeight() - texture.height()) / 2.0));
        gfx.getPixelWriter().setPixels(
                x0, y0, texture.width(), texture.height(),
                javafx.scene.image.PixelFormat.getIntArgbInstance(),
                (IntBuffer) texture.data().clear(),
                texture.width()
        );
    }
}
