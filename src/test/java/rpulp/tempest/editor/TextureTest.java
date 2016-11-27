package rpulp.tempest.editor;

import org.junit.Test;

import static rpulp.tempest.editor.TextureAssert.assertSameTextures;
import static rpulp.tempest.editor.blend.BlendFunctions.SRC_OVERWRITES_DST;

public class TextureTest {

    @Test
    public void clipBottomRight() {
        Texture dst = Texture.create(4, 4);
        Texture src = Texture.create(3, 3);
        dst.clear(1);
        src.clear(2);
        dst.set(2, 2, src, SRC_OVERWRITES_DST);
        assertSameTextures(
                TextureAssert.createTexture(
                        new int[] {1, 1, 1, 1},
                        new int[] {1, 1, 1, 1},
                        new int[] {1, 1, 2, 2},
                        new int[] {1, 1, 2, 2}
                ),
                dst);
    }

    @Test
    public void clipTopLeft() {
        Texture dst = Texture.create(4, 4);
        Texture src = Texture.create(3, 3);
        dst.clear(1);
        src.clear(2);
        dst.set(-1, -2, src, SRC_OVERWRITES_DST);
        assertSameTextures(
                TextureAssert.createTexture(
                        new int[] {2, 2, 1, 1},
                        new int[] {1, 1, 1, 1},
                        new int[] {1, 1, 1, 1},
                        new int[] {1, 1, 1, 1}
                ),
                dst);
    }
}