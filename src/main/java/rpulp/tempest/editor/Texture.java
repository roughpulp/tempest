package rpulp.tempest.editor;

import rpulp.tempest.editor.blend.BlendFunction;

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

    public int getPixel(int x, int y) {
        return data.get(at(x, y));
    }

    public void putPixel(int x, int y, int pixel) {
        data.put(at(x, y), pixel);
    }

    public void putPixelSafe(int x, int y, int pixel) {
        if ((x >= 0) && x < width && (y >= 0) && (y < height)) {
            putPixel(x, y, pixel);
        }
    }

    public void fill(int col) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                data.put(at(x, y), col);
            }
        }
    }

/*
    ...........          ..............
    ...........          ..............
    ...*XXXX...          ..............
    ...XXXXX...          .......*XXXXXX,,,,
    ...XXXXX...          .......XXXXXXX,,,,
    ...........          .......XXXXXXX,,,,
                                ,,,,,,,,,,,
         +,,,,,,,
         ,,,,,,,,
    .....*XXXX,,,        ..............
    .....XXXXX,,,        ..............
    .....XXXXX,,,        ..............
 +,,*XXX......         +,*XXXXXXXXXXXXX,,,,
 ,,,XXXX......         ,,XXXXXXXXXXXXXX,,,,
 ,,,XXXX......         ,,XXXXXXXXXXXXXX,,,,
 ,,,,,,,               ,,,,,,,,,,,,,,,,,,,,
                       ,,,,,,,,,,,,,,,,,,,,

*/
    public void set(int dstX0, int dstY0, Texture src, BlendFunction blend) {
        if (dstX0 >= width || dstY0 >= height) {
            return;
        }
        int srcX0 = 0;
        int srcY0 = 0;
        int srcW = src.width;
        int srcH = src.height;
        if (dstX0 < 0) {
            srcX0 -= dstX0;
            srcW += dstX0;
            dstX0 = 0;
        }
        if (dstY0 < 0) {
            srcY0 -= dstY0;
            srcH += dstY0;
            dstY0 = 0;
        }
        if ((dstX0 + srcW) > width) {
            srcW = width - dstX0;
        }
        if ((dstY0 + srcH) > height) {
            srcH = height - dstY0;
        }
        for (int y = 0; y < srcH; ++y) {
            int srcIdxY = (srcY0 + y) * src.width;
            int dstIdxY = (dstY0 + y) * width;
            for (int x = 0; x < srcW; ++x) {
                int srcIdx = srcIdxY + (srcX0 + x);
                int dstIdx = dstIdxY + (dstX0 + x);
                int srcPixel = src.data.get(srcIdx);
                int dstPixel = data.get(dstIdx);
                int resPixel = blend.blend(srcPixel, dstPixel);
                data.put(dstIdx, resPixel);
            }
        }
    }

    private int at(int x, int y) { return (y * width) + x; }
}
