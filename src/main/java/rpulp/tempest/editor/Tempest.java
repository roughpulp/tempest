package rpulp.tempest.editor;

import javafx.application.Application;
import javafx.scene.Cursor;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.layout.StackPane;
import javafx.stage.Stage;

public class Tempest extends Application{

    @Override
    public void start(Stage stage) throws Exception {

        int width = 1024;
        int height = 768;

        Layers layers = new Layers(width, height);
        for (int ii = 0; ii < 1; ++ii) {
            layers.addLayer();
        }
        layers.setActive(0);
        Canvas canvas = new Canvas(width, height);
        CanvasBridge bridge = CanvasBridge.create(canvas, layers);
        bridge.updateCanvas();
        bridge.setEventHandlers();

        StackPane root = new StackPane();
        root.getChildren().add(canvas);
        Scene scene = new Scene(root, width, height);
        scene.setCursor(Cursor.CROSSHAIR);
        stage.setTitle("Tempest");
        stage.setScene(scene);
        stage.show();

    }

    public static void main(String... args) throws Exception {
        launch(args);
    }
}
