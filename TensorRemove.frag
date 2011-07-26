uniform sampler2D previousTexUnit;
uniform sampler2D stepTexUnit;

void main(void)
{
    vec4 previous = texture2D (previousTexUnit, gl_TexCoord[0].st);
    vec4 step = texture2D (stepTexUnit, gl_TexCoord[0].st);
    gl_FragColor = previous - step;
}

// Local Variables:
// mode: c++
// End:
