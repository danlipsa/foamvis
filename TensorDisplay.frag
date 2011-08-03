#version 120
/**
 * @file   TensorDisplay.frag
 * @author Dan R. Lipsa
 *
 * Displays deformation ellipses
 */

uniform vec2 gridTranslation;
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

struct TensorAc
{
    mat2 m_a;
    float m_c;
};

TensorAc getTensorAc ()
{
    const Tensor t = Tensor (1./25., 1./4., vec2 (1., 0.), vec2 (0., 1.));
    mat2 r = mat2(t.m_v1, t.m_v2);
    mat2 d = mat2 (t.m_l1, 0., 0., t.m_l2);
    mat2 a = r * d * transpose (r);
    return TensorAc (a, min (t.m_l1, t.m_l2) / 4);
}

void main(void)
{
    const vec4 inkColor = vec4 (0., 0., 0., 1.);
    float perc = (cellLength - lineWidth) / cellLength;
    TensorAc t = getTensorAc ();
    float cMax = t.m_c;
    float cMin = perc * perc * cMax;
    vec2 x = (objectCoord - gridTranslation) / cellLength;
    x = fract (x);
    vec2 v = vec2 (0.5, 0.5);
    float value = dot (x - v, t.m_a * (x - v));

    bool backgroundEllipse = value < cMin || cMax < value;
    vec2 percentage = vec2 (perc, perc);
    vec2 backgroundBox = step (x, percentage);
    bool finish = backgroundEllipse;
    //&& (backgroundBox.x * backgroundBox.y != 0.);
    if (finish)
	discard;    
    gl_FragColor = inkColor;
}


// Local Variables:
// mode: c++
// End:
