/**
 * @file   ScalarDisplay.h
 * @author Dan R. Lipsa
 * @date  19 Sep. 2011
 *
 * Interface for the ScalarDisplay class
 */

#ifndef __SCALAR_DISPLAY_H__
#define __SCALAR_DISPLAY_H__

#include "Enums.h"
#include "ShaderProgram.h"

// ScalarDisplay
// ======================================================================
/**
 * RGBA : sum, count, min, max
 */
class ScalarDisplay : public ShaderProgram
{
public:
    ScalarDisplay (const char* frag);
    void Bind (GLfloat minValue, GLfloat maxValue,
	       StatisticsType::Enum displayType, AverageCountType::Enum countType,
               GLfloat globalCount);

    // assume the colorbar is alreay bound on texture unit 0
    GLint GetColorMapTexUnit ()
    {
	return 0;
    }
    GLint GetScalarAverageTexUnit ()
    {
	return 1;
    }

private:
    int m_displayTypeLocation;
    int m_countTypeLocation;
    int m_globalCountLocation;
    int m_minValueLocation;
    int m_maxValueLocation;
    int m_colorBarTexUnitLocation;
    int m_scalarAverageTexUnitLocation;
};



#endif //__SCALAR_DISPLAY_H__

// Local Variables:
// mode: c++
// End:
