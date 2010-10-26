/**
 * @file   DisplayFaceAverage.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Interface for the DisplayFaceAverage class
 */

#ifndef __DISPLAY_FACE_AVERAGE_H__
#define __DISPLAY_FACE_AVERAGE_H__

#include "DisplayElement.h"

class Body;
class Foam;
class GLWidget;


class DisplayFaceAverage : public DisplayElement
{
public:
    DisplayFaceAverage (const GLWidget& glWidget) :
	DisplayElement (glWidget)
    {
    }

    void Init (const QSize& size);
    void InitShaders ();
    void Release ();
    void Step (const Foam& foam);
    void Display ();

    void SetValue (GLfloat value)
    {
	m_addShader.setAttributeValue (m_valueAttr, value);
    }
    void SetMinValue (GLfloat minValue)
    {
	m_addShader.setUniformValue (m_minValueAttr, minValue);
    }
    void SetMaxValue (GLfloat maxValue)
    {
	m_addShader.setUniformValue (m_maxValueAttr, maxValue);
    }
    void SetColorBarTexture (GLfloat textureUnit)
    {
	m_addShader.setUniformValue (m_colorBarTextureAttr, textureUnit);
    }


private:
    template<typename displaySameEdges>
    void displayFacesValues (
	const vector<boost::shared_ptr<Body> >& bodies) const;
    void initAddShader ();
    void initTextureShader ();

private:
    /**
     * Stores the sum and count of values
     */
    boost::scoped_ptr<QGLFramebufferObject> m_new;
    /**
     * Stores the sum and count of the previous step.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_old;
    QGLShaderProgram m_addShader;
    int m_valueAttr;
    int m_minValueAttr;
    int m_maxValueAttr;
    int m_colorBarTextureAttr;

    QGLShaderProgram m_textureShader;
};

#endif //__DISPLAY_FACE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
