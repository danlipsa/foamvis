uniform float u_sigma;
uniform float u_intervalMargin;

const float pi = 3.14159265359;

void main(void)
{    
    vec2 x = gl_TexCoord[0].st;
    vec2 middle = vec2 (0.5, 0.5);
    float sumSquares = dot (x - middle, x - middle);
    float k = 1 / (2 * u_sigma * u_sigma * pi) *
	exp (-2 * pow (u_intervalMargin / u_sigma, 2) * sumSquares);
    gl_FragColor = vec4 (k, 1, 0, 0);
}

// Local Variables:
// mode: c++
// End:
