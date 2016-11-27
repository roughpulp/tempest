package rpulp.tempest.editor;

import javafx.event.EventHandler;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.image.PixelWriter;
import javafx.scene.input.MouseEvent;

import java.nio.IntBuffer;

public class CanvasBridge {

    public static CanvasBridge create(Canvas canvas, Layers layers, Brush brush) {
        GraphicsContext gfx = canvas.getGraphicsContext2D();
        PixelWriter writer = gfx.getPixelWriter();
        return new CanvasBridge(canvas, writer, layers, brush);
    }

    private final Canvas canvas;
    private final PixelWriter writer;
    private final Layers layers;
    private final Texture bottom;
    private final EventHandler<MouseEvent> onMousePressed;
    private final EventHandler<MouseEvent> onMouseReleased;
    private final EventHandler<MouseEvent> onMouseDragged;
    private final Brush brush;

    private CanvasBridge(Canvas canvas, PixelWriter writer, Layers layers, Brush brush) {
        this.canvas = canvas;
        this.writer = writer;
        this.layers = layers;
        this.bottom = Texture.create(layers.width(), layers.height());
        this.brush = brush;
        this.onMousePressed = evt -> {
            brush.onPressed(layers.activeTexture(), (int)evt.getX(), (int)evt.getY());
            updateCanvas();
        };
        this.onMouseDragged = evt -> {
            brush.onMouseDragged(layers.activeTexture(), (int)evt.getX(), (int)evt.getY());
            updateCanvas();
        };
        this.onMouseReleased = evt -> {
            brush.onReleased(layers.activeTexture(), (int)evt.getX(), (int)evt.getY());
            updateCanvas();
        };
    }

    public void setEventHandlers() {
        canvas.addEventHandler(MouseEvent.MOUSE_PRESSED, onMousePressed);
        canvas.addEventHandler(MouseEvent.MOUSE_RELEASED, onMouseReleased);
        canvas.addEventHandler(MouseEvent.MOUSE_DRAGGED, onMouseDragged);
    }

    public void unsetEventHandlers() {
        canvas.removeEventFilter(MouseEvent.MOUSE_PRESSED, onMousePressed);
        canvas.removeEventFilter(MouseEvent.MOUSE_RELEASED, onMouseReleased);
        canvas.removeEventFilter(MouseEvent.MOUSE_DRAGGED, onMouseDragged);
    }

    public void updateCanvas() {
        layers.collapse(bottom);
        writer.setPixels(
                0, 0, bottom.width(), bottom.height(),
                javafx.scene.image.PixelFormat.getIntArgbInstance(),
                (IntBuffer) bottom.data().clear(),
                bottom.width()
                );
    }
}
