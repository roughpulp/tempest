package rpulp.tempest.editor.geometry;

import rpulp.tempest.editor.Texture;

import java.util.ArrayDeque;

public class FloodFill {

/*
        ......................
        ........XXXXXXXXX.....
        .......X........X.....
        .......X.........X....
        ....XXX...X...X....X..
        ..XX.....XX...XX...X..
        ..XX....X.........X..
        ....XX........XXXXX...
        .....X.......XX.......
        .....XXXXXXXX.........
*/

    public static void floodFill(Texture texture, int x0, int y0, int fillColor) {
        final int width = texture.width();
        final int height = texture.height();
        if (x0 < 0 || y0 < 0 || x0 >= width || y0 >= height) {
            return;
        }
        final int walkColor = texture.getPixel(x0, y0);
        if (walkColor == fillColor) {
            return;
        }
        texture.putPixel(x0, y0, fillColor);
        final ArrayDeque<Vec2i> queue = new ArrayDeque<>();
        queue.addFirst(new Vec2i(x0, y0));
        while (queue.size() > 0) {
            Vec2i pt = queue.pollFirst();
            if (pt.x + 1 < width) {
                addCoord(texture, queue, pt.x + 1, pt.y, walkColor, fillColor);
            }
            if (pt.x - 1 >= 0) {
                addCoord(texture, queue, pt.x - 1, pt.y, walkColor, fillColor);
            }
            if (pt.y + 1 < height) {
                addCoord(texture, queue, pt.x, pt.y + 1, walkColor, fillColor);
            }
            if (pt.y - 1 >= 0) {
                addCoord(texture, queue, pt.x, pt.y - 1, walkColor, fillColor);
            }
        }
    }

    private static void addCoord(
            Texture texture,
            ArrayDeque<Vec2i> queue,
            int xx, int yy,
            int walkColor, int fillColor) {
        if (texture.getPixel(xx, yy) == walkColor) {
            queue.addFirst(new Vec2i(xx, yy));
            texture.putPixel(xx, yy, fillColor);
        }
    }
}
