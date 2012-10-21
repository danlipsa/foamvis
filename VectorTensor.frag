#define VERSION #version 120
VERSION
struct Rect
{
    vec2 m_low;
    vec2 m_high;
};

// each fragmet receives its object coordinates
varying vec2 v_objectCoordA;

// line width in object coordinates
uniform float u_lineWidth;
uniform float u_sizeRatio;
// deformation tensors are stored here.
uniform sampler2D u_tensorAverageTexUnit;
// scalar averages are stored here (sum, count, min, max). We use count.
// for vectors, the average is (x, y, count, 0).
uniform sampler2D u_scalarAverageTexUnit;
uniform int u_countIndex;
uniform float u_minValue;
uniform float u_maxValue;
uniform sampler1D u_overlayBarTexUnit;
uniform float u_onePixelInObjectSpace;

uniform bool u_sameSize;
uniform bool u_colorMapped;
uniform bool u_clampingShown;
uniform bool u_glyphShown;


/**
 * @{
 * @name Grid
 */
// object coordinates for screen
uniform Rect u_enclosingRect;
// expressed in (e1, e2) axes
uniform vec2 u_gridTranslationE;
uniform bool u_gridShown;
uniform bool u_gridCellCenterShown;
// cell length in object coordinates
uniform float u_gridCellLength;
// @}

/**
 * @{
 * @name Noise for random seeding points for glyphs
 */
uniform float u_noiseStart;
uniform float u_noiseFrequency;
uniform float u_noiseAmplitude;
// @}



#include "noise2D.glsl"

vec2 snoise2 (vec2 x)
{
    return vec2 (snoise (x), snoise (x) + 43);
}

// x is in [0, 1)
bool isOnGrid (vec2 x)
{
    vec2 isBackground;
    float perc = (u_gridCellLength - u_onePixelInObjectSpace) / u_gridCellLength;
    vec2 percentage = vec2 (perc, perc);
    isBackground = step (x, percentage);
    return isBackground.x * isBackground.y == 0.;
}

// x is in [0, 1)
bool isOnGridCellCenter (vec2 x, vec2 f)
{
    float empty = (u_gridCellLength - 2 * u_onePixelInObjectSpace) / u_gridCellLength;
    float halfFull = (1 - empty) / 2;
    vec2 percentage = vec2 (empty, empty);
    x = fract (
	vec2 (0.5, 0.5) + x - 
	halfFull * (vec2(1,1) + u_noiseAmplitude * snoise2 (
                        u_noiseStart + f / u_noiseFrequency)
            ));
    vec2 isOnLine = step (percentage, x);
    return isOnLine.x * isOnLine.y == 1.0;
}

// x is in (-inf, +inf)
bool isOnGridCenter (vec2 x)
{
    x = abs (x);
    float perc = 2 * u_onePixelInObjectSpace / u_gridCellLength;
    vec2 percentage = vec2 (perc, perc);
    vec2 isOnLine = step (x, percentage);
    return isOnLine.x * isOnLine.y == 1.0;
}

// See doc/TensorDisplay.png
void getCoordinates (out vec2 gridCoord, out vec2 texCoordCenter)
{
    vec2 gridTranslationA = mat2 (
	gl_ModelViewMatrix[0].xy, 
	gl_ModelViewMatrix[1].xy) * u_gridTranslationE;

    // expressed in (a1, a2) coordinate system
    gridCoord = (v_objectCoordA - gridTranslationA) / u_gridCellLength;
    vec2 gridCoordFloor = floor (gridCoord);
    vec2 cellCenterA = u_gridCellLength * (gridCoordFloor + vec2 (.5, .5));

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
