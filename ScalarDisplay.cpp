#include "ScalarDisplay.h"

ScalarDisplay::ScalarDisplay (const char* frag) :
    ShaderProgram (0, frag)
{
    m_displayTypeLocation = uniformLocation ("u_displayType");
    m_countTypeLocation = uniformLocation ("u_countType");
    m_globalCountLocation = uniformLocation ("u_globalCount");
    m_minValueLocation = uniformLocation("u_minValue");
    m_maxValueLocation = uniformLocation("u_maxValue");
    m_colorBarTexUnitLocation = uniformLocation("u_colorBarTexUnit");
    m_scalarAverageTexUnitLocation = uniformLocation("u_scalarAverageTexUnit");
}

void ScalarDisplay::Bind (GLfloat minValue, GLfloat maxValue,
			  StatisticsType::Enum displayType, 
                          CountType::Enum countType, GLfloat globalCount)
{
    ShaderProgram::Bind ();
    setUniformValue (m_displayTypeLocation, displayType);
    setUniformValue (m_countTypeLocation, countType);
    setUniformValue (m_globalCountLocation, globalCount);
    setUniformValue (m_minValueLocation, minValue);
    setUniformValue (m_maxValueLocation, maxValue);
    setUniformValue (m_colorBarTexUnitLocation, GetColorMapTexUnit ());
    setUniformValue (m_scalarAverageTexUnitLocation, 
                     GetScalarAverageTexUnit ());
}
