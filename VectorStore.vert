/**
 * @file VectorStore.vert
 * @author Dan R. Lipsa
 *
 * Stores a vector
 */

attribute vec2 vValue;

varying vec4 fValue;

void main(void)
{
    const float MAX_FLOAT = 3.40282e+38;
    gl_Position = ftransform();
    if (vValue[0] == MAX_FLOAT && vValue[1] == MAX_FLOAT)
        fValue = vec4 (0, 0, 0, 0);
    else
        fValue = vec4 (vValue, 1, 0);
}

// Local Variables:
// mode: c++
// End:
