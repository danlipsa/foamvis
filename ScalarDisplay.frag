// this should match StatisticsType::Enum order
// displayType values: 0=average, 1=min, 2=max, 3=count
uniform int displayType;
uniform float minValue;
uniform float maxValue;
uniform sampler1D colorBarTexUnit;
uniform sampler2D resultTexUnit;

void main(void)
{
    vec4 result = texture2D (resultTexUnit, gl_TexCoord[0].st);
    if (result.g == 0.0)
        gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0);
    else
    {
        float value;
        if (displayType == 0)
	    value = result.r / result.g;
        else if (displayType == 1)
	    value = result.b;
        else if (displayType == 2)
	    value = result.a;
        else
	    value = result.g;
        float colorBarTexIndex = (value - minValue) / (maxValue - minValue);
        gl_FragColor = texture1D (colorBarTexUnit, colorBarTexIndex);
    }
}

// Local Variables:
// mode: c++
// End:
