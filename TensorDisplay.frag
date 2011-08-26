#version 120
/**
 * @file   TensorDisplay.frag
 * @author Dan R. Lipsa
 *
 * Displays deformation ellipses
 */

struct Rect
{
    vec2 m_low;
    vec2 m_high;
};

// see Practical Linear Algebra, ch. 9
// BUG: Mac OS X 10.6.8 OpenGL driver does't like structures 
//      that end in a mat2.
struct Ellipse
{
    // r*d*transpose(r)
    mat2 m_a;
    // ellipse cMin, cMax which determine the size of the ellipse and the 
    // width of the line
    vec2 m_c;
};


// object coordinates for screen
uniform Rect u_srcRect;
uniform Rect u_srcTexRect;
uniform vec2 u_gridTranslation;
// cell length in object coordinates
uniform float u_cellLength;
// line width in object coordinates
uniform float u_lineWidth;
uniform float u_ellipseSizeRatio;
// deformation tensors are stored here.
uniform sampler2D u_tensorAverageTexUnit;
// scalar averages are stored here (sum, count, min, max). We use count.
uniform sampler2D u_scalarAverageTexUnit;

// each fragmet receives the object coordinates of the fragment.
varying vec2 v_objectCoord;


const float sqrt2 = 1.41421356237;
const mat2 tensor45 = mat2 (3., 1., 1. / sqrt2, 1./ sqrt2);
const mat2 tensor_b264 = mat2 (
    0.00099301, 0.000340428,
    0.783351, 0.621579);
const mat2 transform45 = mat2 (2., 1., 1., 2.);
const mat2 transform_b264 = mat2 (
    7.4088e-04, 3.1775e-04, 3.1775e-04, 5.9256e-04);

void swap (inout float first, inout float second)
{
    float temp = first;
    first = second;
    second = temp;
}

vec2 getOrthogonal (vec2 v)
{
    return vec2 (v[1], -v[0]);
}

// see Numerical Recipes, ch 5.6
vec2 getQuadraticRoots (float a, float b, float c)
{
    float r = b * b - 4. * a * c;
    if (r < 0.0 || a == 0.0)
	discard;
    r = sqrt (r);
    float q = (-1.0 / 2.0) * (b + sign (b) * r);
    float first = q / a;
    float second = c / q;
    if (first < second)
	swap (first, second);
    return vec2 (first, second);
}

vec2 getEigenVector (float l, mat2 a)
{
    a[0][0] -= l;
    a[1][1] -= l;
    int i = (abs(a[0][0]) < abs(a[1][0])) ? 1 : 0;
    vec2 r = vec2 (-a[i][1] / a[i][0], 1.0);
    r = normalize (r);
    return r;
}

/**
 * returns true if the transform matrix is valid
 */
bool getTransform (out mat2 a, vec2 texCoordCenter)
{
    float count = texture2D (u_scalarAverageTexUnit, texCoordCenter).g;
    if (count == 0.0)
	return false;

    //debug
    //if (count == 1.0)
    //texCoordCenter = vec2 (0.5, 0.5);
    vec4 ta = texture2D (u_tensorAverageTexUnit, texCoordCenter);
    a = mat2 (ta[0], ta[1], ta[2], ta[3]);
    a = a / count;
    return true;
}

Ellipse fromEigen (mat2 t)
{
    mat2 r = mat2 (t[1], getOrthogonal (t[1]));
    vec2 eigenVal = t[0];
    vec2 l = vec2 (1. / (eigenVal[0] * eigenVal[0]), 
		   1. / (eigenVal[1] * eigenVal[1]));
    mat2 d = mat2 (l[0], 0., 0., l[1]);
    mat2 a = r * d * transpose (r);
    float cMax = u_ellipseSizeRatio / u_cellLength;
    float cMin = cMax - u_lineWidth / eigenVal[1] / u_cellLength;
    Ellipse e = Ellipse (a, vec2 (cMin * cMin, cMax * cMax));
    // debug
    //e = Ellipse (mat2 (1./25., 0., 0., 1./9.), vec2 (1./25., 1./9.));
    return e;
}

Ellipse fromTransform (mat2 a)
{
    vec2 eVal = getQuadraticRoots (1.0, -a[0][0] - a[1][1],
				 a[0][0] * a[1][1] - a[0][1] * a[1][0]);
    vec2 eVec = getEigenVector (eVal[0], a);
    return fromEigen (mat2 (eVal, eVec));
}

// x is in [0, 1)
bool isEllipse (vec2 x, vec2 texCoordCenter)
{
    mat2 a;
    if (getTransform (a, texCoordCenter))
    {
	//Ellipse t = fromEigen (tensor_b264);
	//Ellipse t = fromTransform (transform45);
	Ellipse t = fromTransform (a);
	vec2 v = vec2 (0.5, 0.5);
	float value = dot (x - v, t.m_a * (x - v));
	return (t.m_c[0] <= value && value <= t.m_c[1]);
	//debug
	//return true;
    }
    else
	return false;
}

// x is in [0, 1)
bool isGrid (vec2 x)
{
    vec2 isBackground;
    float perc = (u_cellLength - u_lineWidth) / u_cellLength;
    vec2 percentage = vec2 (perc, perc);
    isBackground = step (x, percentage);
    return isBackground.x * isBackground.y == 0.;
}

// x is in [0, 1)
bool isGridCenter (vec2 x)
{
    x = fract (vec2 (0.5, 0.5) + x);
    vec2 isLine;
    float perc = (u_cellLength - u_lineWidth) / u_cellLength;
    vec2 percentage = vec2 (perc, perc);
    isLine = step (percentage, x);
    return isLine.x * isLine.y == 1.0;
}

void main(void)
{
    const vec4 inkColor = vec4 (0., 0., 0., 1.);
    vec2 gridCoordStart = 
	(u_srcRect.m_low + u_srcRect.m_high) / 2. + u_gridTranslation;
    vec2 gridCoord = (v_objectCoord - gridCoordStart) / u_cellLength;
    vec2 gridCoordFract = fract (gridCoord);
    vec2 gridCoordFloor = floor (gridCoord);
    vec2 gridCoordCenter = u_cellLength * (gridCoordFloor + vec2 (.5, .5));
    vec2 screenCoordCenter = gridCoordStart + gridCoordCenter - u_srcRect.m_low;
    vec2 texCoordCenter = 
	screenCoordCenter / (u_srcRect.m_high - u_srcRect.m_low);
    // use coordinates for the enclosing VV 2.
    texCoordCenter = u_srcTexRect.m_low + 
	(u_srcTexRect.m_high - u_srcTexRect.m_low) * texCoordCenter;
    if (isEllipse (gridCoordFract, texCoordCenter) /*|| 
	isGrid (gridCoordFract) ||
	isGridCenter (gridCoordFract)*/)
	gl_FragColor = inkColor;
    else
	discard;
}


// Local Variables:
// mode: c++
// End:
