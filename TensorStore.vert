/**
 * @file ScalarStore.vert
 * @author Dan R. Lipsa
 *
 * Store a 2x2 matrix in a texel row order: m00, m01, m10, m11
 */

attribute vec4 vValue;

varying vec4 fValue;

void main(void)
{
    gl_Position = ftransform();	
    fValue = vValue;
}

// Local Variables:
// mode: c++
// End:
