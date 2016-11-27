package rpulp.tempest.editor;

import javafx.application.Application;
import javafx.geometry.Orientation;
import javafx.geometry.Rectangle2D;
import javafx.scene.Cursor;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.control.ColorPicker;
import javafx.scene.control.Slider;
import javafx.scene.layout.HBox;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.stage.Screen;
import javafx.stage.Stage;
import org.pmw.tinylog.Configurator;
import org.pmw.tinylog.Level;
import org.pmw.tinylog.writers.ConsoleWriter;

public class Tempest extends Application{

    private static void initLogging() {
        Configurator.defaultConfig()
                .writer(new ConsoleWriter())
                .formatPattern("{date:yyyy-MM-dd HH:mm:ss} - {level} [{thread}] {message}")
                .level(Level.DEBUG)
                .activate();
    }

    @Override
    public void start(Stage stage) throws Exception {

        Rectangle2D primaryScreenBounds = Screen.getPrimary().getVisualBounds();
        System.out.println(primaryScreenBounds);

        int width = 1024;
        int height = 1024;

        final Brush brush = new Brush();
        final TextureCanvas brushCanvas = new TextureCanvas(() -> brush.motif(), 128, 128);

        final ColorPicker colorPicker = new ColorPicker();
        colorPicker.setOnAction(evt -> {
            brush.setColor(colorPicker.getValue());
            brushCanvas.updateCanvas();
        });
        colorPicker.setValue(Color.BLACK);
        brush.setColor(Color.BLACK);

        final Slider brushSizeSlider = new Slider();
        brushSizeSlider.setMin(1);
        brushSizeSlider.setMax(100);
        brushSizeSlider.setOrientation(Orientation.HORIZONTAL);
        brushSizeSlider.valueProperty().addListener( (ov, old, neu) -> {
            brush.setSize(neu.intValue());
            brushCanvas.updateCanvas();
        });
        brushSizeSlider.setValue(10);
        brush.setSize(10);

        brushCanvas.updateCanvas();

        Layers layers = new Layers(width, height);
        for (int ii = 0; ii < 1; ++ii) {
            layers.addLayer();
        }
        layers.setActive(0);
        Canvas canvas = new Canvas(width, height);
        CanvasBridge bridge = CanvasBridge.create(canvas, layers, brush);
        bridge.updateCanvas();
        bridge.setEventHandlers();

        final VBox vToolsBox = new VBox();
        vToolsBox.getChildren().addAll(
                colorPicker,
                brushCanvas.canvas(),
                brushSizeSlider
        );

        final HBox hbox = new HBox();
        hbox.getChildren().addAll(vToolsBox, canvas);

        StackPane root = new StackPane();
        root.getChildren().add(hbox);
        Scene scene = new Scene(root, width, height);
        scene.setCursor(Cursor.CROSSHAIR);
        stage.setTitle("Tempest");
        stage.setScene(scene);
        stage.setMaximized(true);
        stage.show();

    }

    public static void main(String... args) throws Exception {
        initLogging();
        launch(args);
    }
}
