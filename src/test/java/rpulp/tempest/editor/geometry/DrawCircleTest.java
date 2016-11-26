package rpulp.tempest.editor.geometry;

import org.junit.Test;
import rpulp.tempest.editor.Texture;

import static rpulp.tempest.editor.TextureAssert.textureToString;

public class DrawCircleTest {

    @Test
    public void drawCircleRadius0() { drawCircle(0); }

    @Test
    public void drawCircleRadius1() { drawCircle(1); }

    @Test
    public void drawCircleRadius6() { drawCircle(6); }

    public void drawCircle(int radius) {
        Texture texture = Texture.create(20, 20);
        texture.fill('.');
        DrawCircle.drawCircle(10, 10, radius, (x, y) -> texture.putPixel(x, y, 'X') );
        System.out.println(textureToString(texture));
    }

}