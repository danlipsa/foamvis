// Vertex shader activated by writing a quad over the entire screen.
// Vertices have to be in object coordinates, translated with the grid
// translation desired.

// each fragmet receives the object coordinates of the fragment.
varying vec2 v_objectCoord;

void main(void)
{
    v_objectCoord = gl_Vertex.xy;
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}

// Local Variables:
// mode: c++
// End:
