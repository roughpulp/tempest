package rpulp.tempest.editor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

public class TextureAssert {

    public static Texture createTexture(int[]... rows) {
        int height = rows.length;
        if (height == 0) {
            return Texture.create(0, 0);
        }
        int width = rows[0].length;
        Texture texture = Texture.create(width, height);
        for(int y = 0; y < height; ++y) {
            assertEquals(width, rows[y].length);
            for (int x = 0; x < width; ++x) {
                texture.data().put((y * width) + x, rows[y][x]);
            }
        }
        return texture;
    }

    public static Texture createTexture(String... rows) {
        int height = rows.length;
        if (height == 0) {
            return Texture.create(0, 0);
        }
        int width = rows[0].length();
        Texture texture = Texture.create(width, height);
        for(int y = 0; y < height; ++y) {
            assertEquals(width, rows[y].length());
            for (int x = 0; x < width; ++x) {
                texture.data().put((y * width) + x, rows[y].charAt(x));
            }
        }
        return texture;
    }

    public static void assertSameTextures(Texture expected, Texture actual) {
        assertEquals(expected.width(), actual.width());
        assertEquals(expected.height(), actual.height());
        for (int y = 0; y < expected.height(); ++y) {
            for (int x = 0; x < expected.width(); ++x) {
                int idx = (y * expected.width()) + x;
                int xPixel = expected.data().get(idx);
                int aPixel = actual.data().get(idx);
                if (xPixel != aPixel) {
                    fail("different pixel at (" + x + ", " + y + ")");
                }
            }
        }
    }

    public static String textureToString(Texture texture) {
        final StringBuilder sb = new StringBuilder();
        for (int yy = 0; yy < texture.height(); ++yy) {
            for (int xx = 0; xx < texture.width(); ++xx) {
                sb.append((char)texture.getPixel(xx, yy));
            }
            sb.append("\n");
        }
        return sb.toString();
    }
}
