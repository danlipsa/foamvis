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
    gl_Position = ftransform();	
    fValue = vec4 (vValue, 1, 0);
}

// Local Variables:
// mode: c++
// End:
