// Vertex shader activated by writing a quad over the entire screen.
// Vertices have to be in object coordinates, translated with the grid
// translation desired.

// each fragmet receives the object coordinates of the fragment.
varying vec2 v_objectCoordA;

uniform vec2 u_rotationCenter;

void main(void)
{
    v_objectCoordA = gl_Vertex.xy - u_rotationCenter;
    // express in  (a1, a2) coordinates.
    v_objectCoordA = 
	mat2 (gl_ModelViewMatrix[0].xy, gl_ModelViewMatrix[1].xy) *
	v_objectCoordA;

    gl_Position = ftransform();
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}

// Local Variables:
// mode: c++
// End:
