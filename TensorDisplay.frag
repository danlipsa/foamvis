/**
 * @file   TensorDisplay.frag
 * @author Dan R. Lipsa
 *
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
    vec4 inkColor = vec4 (0, 0, 0, 1);
    vec4 backgroundColor = vec4 (1, 1, 1, 0);
    vec2 position = objectCoord / cellLength;
    position = fract (position);
    vec2 percentage = vec2 (.8, .8);
    vec2 useBackground = step (position, percentage);
    float finish = useBackground.x * useBackground.y;
    if (finish != 0.0)
	discard;    
    vec4 color = mix (inkColor, backgroundColor, finish);
    gl_FragColor = color;
}

// Local Variables:
// mode: c++
// End:
