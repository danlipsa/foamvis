uniform sampler2D u_gaussianTexUnit;

void main(void)
{
    float value = texture2D (u_gaussianTexUnit, gl_TexCoord[0].st)[0];
    gl_FragColor = vec4 (value, 1, value, value);
}

// Local Variables:
// mode: c++
// End:
