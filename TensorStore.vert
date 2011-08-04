/**
 * @file ScalarStore.vert
 * @author Dan R. Lipsa
 *
 * Store a mat2 in column order.
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
