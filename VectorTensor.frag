#define VERSION #version 120
VERSION
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
uniform float u_noiseStart;
uniform float u_noiseFrequency;
uniform float u_sizeRatio;
// deformation tensors are stored here.
uniform sampler2D u_tensorAverageTexUnit;
// scalar averages are stored here (sum, count, min, max). We use count.
uniform sampler2D u_scalarAverageTexUnit;
uniform bool u_gridShown;
uniform bool u_gridCellCenterShown;
uniform bool u_clampingShown;
uniform float u_onePixelInObjectSpace;

// x is in [0, 1)
bool isOnGrid (vec2 x)
{
    vec2 isBackground;
    float perc = (u_cellLength - u_onePixelInObjectSpace) / u_cellLength;
    vec2 percentage = vec2 (perc, perc);
    isBackground = step (x, percentage);
    return isBackground.x * isBackground.y == 0.;
}

// x is in [0, 1)
bool isOnGridCellCenter (vec2 x, vec2 f)
{
    float p = (u_cellLength - 2 * u_onePixelInObjectSpace) / u_cellLength;
    vec2 percentage = vec2 (p, p);
    vec2 s = (vec2 (1, 1) - percentage) / 2;
    x = fract (vec2 (0.5, 0.5) + x - s - 
	       13 * s * noise2 (u_noiseStart + f / u_noiseFrequency));
    vec2 isOnLine = step (percentage, x);
    return isOnLine.x * isOnLine.y == 1.0;
}

// x is in (-inf, +inf)
bool isOnGridCenter (vec2 x)
{
    x = abs (x);
    float perc = 2*u_onePixelInObjectSpace / u_cellLength;
    vec2 percentage = vec2 (perc, perc);
    vec2 isOnLine = step (x, percentage);
    return isOnLine.x * isOnLine.y == 1.0;
}

// See doc/TensorDisplay.pdf
void getCoordinates (out vec2 gridCoord, out vec2 texCoordCenter)
{
    vec2 gridTranslationA = mat2 (
	gl_ModelViewMatrix[0].xy, 
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


// VectorTensor.frag
// Local Variables:
// mode: c++
// End:
