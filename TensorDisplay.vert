// Vertex shader activated by writing a quad over the entire screen.
// Vertices have to be in object coordinates, translated with the grid
// translation desired.

// each fragmet receives the object coordinates of the fragment.
varying vec2 objectCoord;

void main(void)
{
    objectCoord = gl_Vertex.xy - foamLow;
    gl_Position = ftransform();
}

// Local Variables:
// mode: c++
// End:
