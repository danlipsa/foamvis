/**
 * @file ScalarStore.vert
 * @author Dan R. Lipsa
 *
 * Store a scalar value in a texture
 */

attribute float vValue;

varying float fValue;

void main(void)
{
    gl_Position = ftransform();	
    fValue = vValue;
}

// Local Variables:
// mode: c++
// End:
