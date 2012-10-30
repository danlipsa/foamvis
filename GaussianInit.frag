uniform float u_sigma;
uniform float u_intervalMargin;


const float pi = 3.14159265359;

void main(void)
{    
    vec2 x = gl_TexCoord[0].st;
    vec2 middle = vec2 (0.5, 0.5);
    float sumSquares = dot (x - middle, x - middle);
    float k = 1.0 / (u_sigma * sqrt (2.0 * pi)) *
	exp (- 0.5 * pow (u_intervalMargin / u_sigma, 2.0) * sumSquares);
    gl_FragColor = vec4 (k, 0, 0, 0);
}

// Local Variables:
// mode: c++
// End:
