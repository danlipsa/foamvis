#include "ScalarDisplay.h"

ScalarDisplay::ScalarDisplay (const char* frag) :
    ShaderProgram (0, frag)
{
    m_displayTypeLocation = uniformLocation ("u_displayType");
    m_minValueLocation = uniformLocation("u_minValue");
    m_maxValueLocation = uniformLocation("u_maxValue");
    m_colorBarTexUnitLocation = uniformLocation("u_colorBarTexUnit");
    m_scalarAverageTexUnitLocation = uniformLocation("u_scalarAverageTexUnit");
}

void ScalarDisplay::Bind (GLfloat minValue, GLfloat maxValue,
			  StatisticsType::Enum displayType)
{
    ShaderProgram::Bind ();
    setUniformValue (m_displayTypeLocation, displayType);
    setUniformValue (m_minValueLocation, minValue);
    setUniformValue (m_maxValueLocation, maxValue);
    setUniformValue (m_colorBarTexUnitLocation, GetColorBarTexUnit ());
    setUniformValue (m_scalarAverageTexUnitLocation, 
                     GetScalarAverageTexUnit ());
}
