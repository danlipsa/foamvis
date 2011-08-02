#version 120
/**
 * @file   TensorDisplay.frag
 * @author Dan R. Lipsa
 *
 * Displays deformation ellipses
 */

// cell length in object coordinates
uniform float cellLength;
// line width in object coordinates
uniform float lineWidth;
// deformation tensors are stored here.
uniform sampler2D tensorAverageTexUnit;
// scalar averages are stored here (sum, count, min, max). We use count.
uniform sampler2D scalarAverageTexUnit;

// each fragmet receives the object coordinates of the fragment.
varying vec2 objectCoord;

struct Tensor
{
    float m_l1;
    float m_l2;
    vec2 m_v1;
    vec2 m_v2;
};

Tensor calculateTensor ()
{
    return Tensor (2., 1., vec2 (1., 0.), vec2 (0., 1.));
}

void main(void)
{
    const vec4 inkColor = vec4 (0., 0., 0., 1.);
    Tensor t = calculateTensor ();
    float c = max (t.m_l1, t.m_l2) / 4.;
    vec2 x = objectCoord / cellLength;
    x = fract (x);
    mat2 r = mat2(t.m_v1, t.m_v2);
    mat2 d = mat2 (t.m_l1, 0., 0., t.m_l2);
    float value = dot (x, r * d * transpose (r) * x);

    float perc = (cellLength - lineWidth) / cellLength;
    vec2 percentage = vec2 (perc, perc);
    vec2 useBackground = step (x, percentage);
    float finish = useBackground.x * useBackground.y;
    if (finish != 0.0)
	discard;    
    gl_FragColor = inkColor;
}


// Local Variables:
// mode: c++
// End:
