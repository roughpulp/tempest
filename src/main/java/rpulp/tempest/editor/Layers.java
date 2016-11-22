package rpulp.tempest.editor;

public class Layers {

    private int width;
    private int height;
    private Texture[] textures = new Texture[16];
    private int nbLayers = 0;
    private int active = -1;

    public Layers(int width, int height) {
        this.width = width;
        this.height = height;
    }

    public int width() { return width; }

    public int height() { return height; }

    public void addLayer() {
        if (nbLayers >= textures.length) {
            final Texture[] old = textures;
            textures = new Texture[(nbLayers * 3) / 2 + 1];
            System.arraycopy(old, 0, textures, 0, old.length);
        }
        textures[nbLayers] = Texture.create(width, height);
        ++nbLayers;
    }

    public void setActive(int active) {
        this.active = active;
    }

    public Texture activeTexture() { return textures[active]; }

    public void collapse(Texture dst) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < height; ++x) {
                int idx = (y * width) + x;
                int col = 0;
                for (int ll = nbLayers - 1; ll >= 0; --ll) {
                    col += textures[ll].data().get(idx);
                }
                dst.data().put(idx, col);
            }
        }
    }
}
