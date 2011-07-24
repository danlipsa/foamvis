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
