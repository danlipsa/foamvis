/*
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

void main(void)
{
    
}

// Local Variables:
// mode: c++
// End:
