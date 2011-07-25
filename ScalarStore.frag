/**
 * Shader that stores a floating point value in a floating point texture:
 * RGBA: value, 1, value, value
 *
 */
varying float fValue;

void main(void)
{
    float maxFloat = 3.40282e+38;
    if (fValue == maxFloat)
        gl_FragColor = vec4 (0, 0, maxFloat, -maxFloat);
    else
        gl_FragColor = vec4 (fValue, 1, fValue, fValue);
}

// Local Variables:
// mode: c++
// End:
