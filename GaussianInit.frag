uniform float u_sigma;
const float pi = 3.14159265359;

void main(void)
{   
    // WARNING: has to be the same as in T1KDE2D.cpp
    const float STDDEV_COUNT = 5.0;
    float interval = STDDEV_COUNT * u_sigma;
    // x varies between [-0.5, 0.5]
    vec2 x = gl_TexCoord[0].st - vec2 (0.5, 0.5);
    // x varies between [-interval/2, interval/2]
    x = interval * x;
    float sumSquares = dot (x, x);
    //float peakHeight = 1.0 / (u_sigma * u_sigma * 2.0 * pi);
    float peakHeight = 1;
    float k = peakHeight * exp (- 0.5 * sumSquares / (u_sigma * u_sigma));
    gl_FragColor = vec4 (k, 0, 0, 0);
}

// Local Variables:
// mode: c++
// End:
