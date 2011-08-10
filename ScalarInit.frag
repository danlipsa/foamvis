void main(void)
{
    const float maxFloat = 3.40282e+38;
    gl_FragColor = vec4 (0, 0, maxFloat, -maxFloat);
}

// Local Variables:
// mode: c++
// End:
