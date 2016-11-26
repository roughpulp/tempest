package rpulp.tempest.editor.geometry;

public class Vec2i {
    public int x;
    public int y;

    public Vec2i() {}

    public Vec2i(int x, int y) {
        this.x = x;
        this.y = y;
    }

    @Override
    public String toString() {
        return "(" + x + ", " + y + ")";
    }

    @Override
    public boolean equals(Object other) {
        if (! (other instanceof Vec2i)) {
            return false;
        }
        Vec2i that = (Vec2i)other;
        return x == that.x && y == that.y;
    }

    @Override
    public int hashCode() {
        return (y << 16) + x;
    }
}
