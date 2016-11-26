package rpulp.tempest.editor.blend;

public interface BlendFunction {
    /**
     *
     * @param src incoming pixel
     * @param dst pixel already in the texture
     */
    int blend(int src, int dst);
}
