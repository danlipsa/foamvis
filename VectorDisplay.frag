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


// each fragmet receives the object coordinates of the fragment.
varying vec2 v_objectCoordA;

// object coordinates for screen
uniform Rect u_enclosingRect;
// expressed in (e1, e2) axes
uniform vec2 u_gridTranslationE;
// cell length in object coordinates
uniform float u_cellLength;
// line width in object coordinates
uniform float u_lineWidth;
uniform float u_sizeRatio;
// deformation tensors are stored here.
uniform sampler2D u_tensorAverageTexUnit;
// scalar averages are stored here (sum, count, min, max). We use count.
uniform sampler2D u_scalarAverageTexUnit;
uniform bool u_gridShown;
uniform bool u_gridCellCenterShown;


float lineWidthPerc = u_lineWidth / u_cellLength;


/**
 * @return true if the transform matrix is valid
 */
bool getVector (vec2 texCoordCenter, out vec2 v)
{
    float count = texture2D (u_scalarAverageTexUnit, texCoordCenter).g;
    if (count == 0.0)
	return false;
    v = texture2D (u_tensorAverageTexUnit, texCoordCenter).xy;
    v = v / count;
    return true;
}


void rotateVector (inout vec2 v)
{
    mat2 r = mat2 (gl_ModelViewMatrix[0].xy, 
		   gl_ModelViewMatrix[1].xy);
    v = r * v;
}

bool isRectangle (vec2 x, vec2 n, float width, 
		  float lengthLeft, float lengthRight)
{
    width = width / 2;
    lengthLeft = lengthLeft / 2;
    lengthRight = lengthRight / 2;
    return 
	// lines along n
	dot (x, vec2 (n[0], n[1])) <= width 
	&& dot (x, vec2 (-n[0], -n[1])) <= width 
	// lines perpendicular to n
	&& dot (x, vec2 (-n[1], n[0])) <= lengthLeft
	&& dot (x, vec2 (n[1], -n[0])) <= lengthRight;
}


// x is in [0, 1)
bool isArrow (vec2 x, vec2 texCoordCenter)
{
    vec2 v;
    if (getVector (texCoordCenter, v))
    {
	x = x - vec2 (0.5, 0.5);
	rotateVector (v);
	v = v * u_sizeRatio / u_cellLength;
	vec2 n = normalize (v);
	n = vec2 (-n[1], n[0]);
	float magnitude = length (v);
	return isRectangle (x, n, 3 * lineWidthPerc, magnitude, 0) ||
	    isRectangle (x, n, lineWidthPerc, 0, magnitude);
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
bool isGridCellCenter (vec2 x)
{
    x = fract (vec2 (0.5, 0.5) + x);
    vec2 isLine;
    float p = (u_cellLength - u_lineWidth) / u_cellLength;
    vec2 percentage = vec2 (p, p);
    isLine = step (percentage, x);
    return isLine.x * isLine.y == 1.0;
}

// x is in (-inf, +inf)
bool isGridCenter (vec2 x)
{
    x = abs (x);
    vec2 isLine;
    float perc = 3*u_lineWidth / u_cellLength;
    vec2 percentage = vec2 (perc, perc);
    isLine = step (x, percentage);
    return isLine.x * isLine.y == 1.0;
}

// See doc/TensorDisplay.pdf
void getCoordinates (out vec2 gridCoord, out vec2 texCoordCenter)
{
    vec2 gridTranslationA = mat2 (gl_ModelViewMatrix[0].xy, 
				  gl_ModelViewMatrix[1].xy) * u_gridTranslationE;

    // expressed in (a1, a2) coordinate system
    gridCoord = (v_objectCoordA - gridTranslationA) / u_cellLength;
    vec2 gridCoordFloor = floor (gridCoord);
    vec2 cellCenterA = u_cellLength * (gridCoordFloor + vec2 (.5, .5));

    // express in  (e1, e2) coordinates.
    vec2 cellCenterE = mat2 (gl_ModelViewMatrixInverse[0].xy, 
		       gl_ModelViewMatrixInverse[1].xy) * cellCenterA;
    texCoordCenter = 
	(cellCenterE + u_gridTranslationE - u_enclosingRect.m_low) / 
	(u_enclosingRect.m_high - u_enclosingRect.m_low);
}




void main (void)
{
    const vec4 inkColor = vec4 (0., 0., 0., 1.);
    vec2 gridCoord, texCoordCenter;
    getCoordinates (gridCoord, texCoordCenter);
    vec2 gridCoordFract = fract (gridCoord);
    if (isArrow (gridCoordFract, texCoordCenter) || 
	(u_gridShown && isGrid (gridCoordFract)) ||
	(u_gridCellCenterShown && isGridCellCenter (gridCoordFract)) ||
	isGridCenter (gridCoord)
	)
	gl_FragColor = inkColor;
    else
	discard;
}


// Local Variables:
// mode: c++
// End:
