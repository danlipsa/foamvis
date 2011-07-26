/**
 * Shader that stores a 2x2 matrix in a texel, column
 * RGBA: m00, m10, m01, m11
 *
 */
varying vec4 fValue;

void main(void)
{
    float maxFloat = 3.40282e+38;
    if (fValue[0] == maxFloat)
        gl_FragColor = vec4 (0);
    else
        gl_FragColor = fValue;
}

// Local Variables:
// mode: c++
// End:
