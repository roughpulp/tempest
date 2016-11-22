package rpulp.tempest.editor;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;

public class Texture
{
    public static Texture create(int width, int height) {
        int size = width * height * 4;
        ByteBuffer data = ByteBuffer.allocateDirect(size);
        return new Texture(width, height, data.asIntBuffer());
    }

    private final int width;
    private final int height;
    private final IntBuffer data;

    private Texture(int width, int height, IntBuffer data) {
        this.width = width;
        this.height = height;
        this.data = data;
    }

    public int width() { return width; }

    public int height() { return height; }

    public IntBuffer data() { return data; }

    public int getArgb(int x, int y) {
        return data.get(at(x, y));
    }

    public void setArgb(int x, int y, int argb) {
        data.put(at(x, y), argb);
    }

    private int at(int x, int y) { return (y * width) + x; }
}
