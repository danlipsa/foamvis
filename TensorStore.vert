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
