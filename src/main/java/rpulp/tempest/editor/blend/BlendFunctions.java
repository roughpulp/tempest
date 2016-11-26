package rpulp.tempest.editor.blend;

public class BlendFunctions {

    public static BlendFunction SRC_OVERWRITES_DST = (src, dst) -> src;

    public static BlendFunction SRC_ALPHA_ONE_MINUS_SRC_ALPHA = (src, dst) -> {
        float sA = (float)((src >>> (8 * 3)) & 0xff) / 255.0f;
        float sR = (float)((src >>> (8 * 2)) & 0xff) / 255.0f;
        float sG = (float)((src >>> (8 * 1)) & 0xff) / 255.0f;
        float sB = (float)((src >>> (8 * 0)) & 0xff) / 255.0f;
        //
        float dA = (float)((dst >>> (8 * 3)) & 0xff) / 255.0f;
        float dR = (float)((dst >>> (8 * 2)) & 0xff) / 255.0f;
        float dG = (float)((dst >>> (8 * 1)) & 0xff) / 255.0f;
        float dB = (float)((dst >>> (8 * 0)) & 0xff) / 255.0f;

        float rA = (sA * sA) + (dA * (1.0f - sA));
        float rR = (sA * sR) + (dR * (1.0f - sA));
        float rG = (sA * sG) + (dG * (1.0f - sA));
        float rB = (sA * sB) + (dB * (1.0f - sA));

        int res = (byt(rA) << (8 * 3))
                | (byt(rR) << (8 * 2))
                | (byt(rG) << (8 * 1))
                | (byt(rB) << (8 * 0))
                ;
        return res;
    };

    private static int byt(float v) { return (int)(v * 255.0f); }
}
