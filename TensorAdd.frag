/**
 * Shader program that performs the following operation: 
 * current = previous op step
 * where current, previous and step are floating point textures and op is + or -
 */
uniform sampler2D previousTexUnit;
uniform sampler2D stepTexUnit;

void main(void)
{
    vec4 previous = texture2D (previousTexUnit, gl_TexCoord[0].st);
    vec4 step = texture2D (stepTexUnit, gl_TexCoord[0].st);
    //previous = vec4 (0.0, 0.0, 0.0, 1.0);
    gl_FragColor = previous + step;
}
// Local Variables:
// mode: c++
// End:
