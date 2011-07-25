/**
 * Shader program that performs the following operation: 
 * current = previous op step
 * where current, previous and step are floating point textures and op is + or -
 * RGBA : sum, count, min, max
 */
uniform sampler2D previousTexUnit;
uniform sampler2D stepTexUnit;

void main(void)
{
    vec4 previous = texture2D (previousTexUnit, gl_TexCoord[0].st);
    vec4 step = texture2D (stepTexUnit, gl_TexCoord[0].st);
    vec2 currentSumCount = previous.rg + step.rg;
    float min = min (previous.b, step.b);
    float max = max (previous.a, step.a);
    gl_FragColor = vec4 (currentSumCount, min, max);
}
// Local Variables:
// mode: c++
// End:
