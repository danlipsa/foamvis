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
    // eigen values
    vec2 m_l;
    // eigen vectors in column order
    mat2 m_a;
};

// see Practical Linear Algebra, ch. 9
struct Ellipse
{
    // ellipse coefficients = 1 / (eigen value)^2
    vec2 m_l;
    // r*d*transpose(r)
    mat2 m_a;
};

const Tensor tensor45 = Tensor (vec2 (3., 1.), 
				mat2 (1., 1., 1., -1.) / 1.4142);
const mat2 transform45 = mat2 (2., 1., 1., 2.);

Ellipse fromEigen (Tensor t)
{
    mat2 r = t.m_a;
    vec2 ec = vec2 (1 / (t.m_l[0] * t.m_l[0]), 1 / (t.m_l[1], t.m_l[1]));
    mat2 d = mat2 (ec[0], 0., 0., ec[1]);
    mat2 a = r * d * transpose (r);
    return Tensor (ec, a);
}

// see Numerical Recipes, ch 5.6
vec2 getQuadraticRoots (float a, float b, float c)
{
    float r = b * b - 4. * a * c;
    if (r < 0.0 || a == 0.0)
	discard;
    r = sqrt (r);
    float q = (-1 / 2) * (b + sign (b) * r);
    return vec2 (q / a, c / q);
}

mat2 getTransform ()
{
    vec4 scalarAverage = texture2D (scalarAverageTexUnit, gl_TexCoord[0].st);
    vec4 ta = texture2D (tensorAverageTexUnit, gl_TexCoord[0].st);
    mat2 a = mat2 (ta[0], ta[1], ta[2], ta[3]);
    a = a / scalarAverage.g;
    return a;
}

Ellipse fromTransform (mat2 a)
{
    vec2 l = getQuadraticRoots (1.0, -a[0][0] - a[1][1],
				a[0][0] * a[1][1] - a[0][1] * a[1][0]);
    return Ellipse (l, a);
}

void main(void)
{
    const vec4 inkColor = vec4 (0., 0., 0., 1.);
    float perc = (cellLength - lineWidth) / cellLength;
    Ellipse t = fromEigen (tensor45);
    //Tensor t = fromTransform (getTransform ());
    //Tensor t = fromTransform (transform45);
    float cMax = min (t.m_l[0], t.m_l[1]) / 4.;
    float cMin = perc * perc * cMax;
    vec2 x = (objectCoord - gridTranslation) / cellLength;
    x = fract (x);
    vec2 v = vec2 (0.5, 0.5);
    float value = dot (x - v, t.m_a * (x - v));
    bool backgroundEllipse = value < cMin || cMax < value;
    vec2 percentage = vec2 (perc, perc);
    vec2 backgroundBox = step (x, percentage);
    bool finish = backgroundEllipse
	&& (backgroundBox.x * backgroundBox.y != 0.);
    if (finish)
	discard;    
    gl_FragColor = inkColor;
}


// Local Variables:
// mode: c++
// End:
