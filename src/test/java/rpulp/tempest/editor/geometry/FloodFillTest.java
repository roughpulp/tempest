package rpulp.tempest.editor.geometry;

import org.junit.Test;
import rpulp.tempest.editor.Texture;
import rpulp.tempest.editor.TextureAssert;

import static rpulp.tempest.editor.TextureAssert.createTexture;
import static rpulp.tempest.editor.TextureAssert.textureToString;


public class FloodFillTest {

    @Test
    public void floodFill(){
        Texture texture = createTexture(
                "                            ",
                " ..............             ",
                " .             ......       ",
                "  .......            ...    ",
                ".........               ..  ",
                ".                        .  ",
                "     ....     .......    .  ",
                "    .    .    .   ..     .  ",
                ".   .    .    ....       .  ",
                " .   ....               .   ",
                "  .                    .    ",
                "   ....    ..      ....     ",
                "       ............         "
        );
        FloodFill.floodFill(texture, 10, 9, '!');
        System.out.println(textureToString(texture));
        TextureAssert.assertSameTextures(
                createTexture(
                        "                            ",
                        " ..............             ",
                        " .!!!!!!!!!!!!!......       ",
                        "  .......!!!!!!!!!!!!...    ",
                        ".........!!!!!!!!!!!!!!!..  ",
                        ".!!!!!!!!!!!!!!!!!!!!!!!!.  ",
                        "!!!!!....!!!!!.......!!!!.  ",
                        "!!!!.    .!!!!.   ..!!!!!.  ",
                        ".!!!.    .!!!!....!!!!!!!.  ",
                        " .!!!....!!!!!!!!!!!!!!!.   ",
                        "  .!!!!!!!!!!!!!!!!!!!!.    ",
                        "   ....!!!!..!!!!!!....     ",
                        "       ............         "
                ),
                texture
        );
    }
    
}