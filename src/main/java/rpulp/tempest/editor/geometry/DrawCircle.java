package rpulp.tempest.editor.geometry;

public class DrawCircle {
    public static void drawCircle(
            int x0, int y0,
            int radius,
            Action action) {
        int x = radius;
        int y = 0;
        int err = 0;

        while (x >= y)
        {
            action.at(x0 + x, y0 + y);
            action.at(x0 + y, y0 + x);
            action.at(x0 - y, y0 + x);
            action.at(x0 - x, y0 + y);
            action.at(x0 - x, y0 - y);
            action.at(x0 - y, y0 - x);
            action.at(x0 + y, y0 - x);
            action.at(x0 + x, y0 - y);
            y += 1;
            err += 1 + (y * 2);
            if (2 * (err - x) + 1 > 0)
            {
                x -= 1;
                err += 1 - 2*x;
            }
        }
    }
}