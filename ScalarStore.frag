/**
 * Shader that stores a floating point value in a floating point texture:
 * RGBA: value, 1, value, value
 * RGBA: sum, count, min, max
 *
 */
varying float fValue;

void main(void)
{
    const float MAX_FLOAT = 3.40282e+38;
    if (fValue == MAX_FLOAT)
        gl_FragColor = vec4 (0, 0, MAX_FLOAT, -MAX_FLOAT);
    else
        gl_FragColor = vec4 (fValue, 1, fValue, fValue);
}

// Local Variables:
// mode: c++
// End:
