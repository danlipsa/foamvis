/*
 * Displays a color-mapped scalar average, min, max or count.
 */

// This should match StatisticsType::Enum
// displayType possible values: 0=average, 1=min, 2=max, 3=count
uniform int u_displayType;
uniform float u_minValue;
uniform float u_maxValue;
uniform sampler1D u_colorBarTexUnit;
// RGBA: sum, count, min, max
uniform sampler2D u_scalarAverageTexUnit;

void main(void)
{
    vec4 result = texture2D (u_scalarAverageTexUnit, gl_TexCoord[0].st);
    if (result.g == 0.0)
        gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0);
    else
    {
        float value;
        if (u_displayType == 0)
	    value = result.r / result.g;
        else if (u_displayType == 1)
	    value = result.b;
        else if (u_displayType == 2)
	    value = result.a;
        else
	    value = result.g;
        float colorBarTexIndex = (value - u_minValue) / 
	    (u_maxValue - u_minValue);
        gl_FragColor = texture1D (u_colorBarTexUnit, colorBarTexIndex);
    }
}

// Local Variables:
// mode: c++
// End:
