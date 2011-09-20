#include "ScalarDisplay.h"

ScalarDisplay::ScalarDisplay (const char* frag) :
    ShaderProgram (0, frag)
{
    m_displayTypeLocation = uniformLocation ("displayType");
    m_minValueLocation = uniformLocation("minValue");
    m_maxValueLocation = uniformLocation("maxValue");
    m_colorBarTexUnitLocation = uniformLocation("colorBarTexUnit");
    m_scalarAverageTexUnitLocation = uniformLocation("scalarAverageTexUnit");
}

void ScalarDisplay::Bind (GLfloat minValue, GLfloat maxValue,
				 StatisticsType::Enum displayType)
{
    ShaderProgram::Bind ();
    setUniformValue (m_displayTypeLocation, displayType);
    setUniformValue (m_minValueLocation, minValue);
    setUniformValue (m_maxValueLocation, maxValue);
    setUniformValue (m_colorBarTexUnitLocation, GetColorBarTexUnit ());
    setUniformValue (m_scalarAverageTexUnitLocation, GetScalarAverageTexUnit ());
}
