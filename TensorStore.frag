/**
 * Shader that stores a mat2 in column order
 * RGBA: m00, m10, m01, m11
 *
 */
varying vec4 fValue;

void main(void)
{
    gl_FragColor = fValue;
}

// Local Variables:
// mode: c++
// End:
