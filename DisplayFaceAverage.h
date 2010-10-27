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
    void Display (GLfloat minValue, GLfloat maxValue,
		  GLint colorBarTexUnit);
    void Calculate (BodyProperty::Enum bodyProperty);

private:
    void step (const Foam* foam = 0, size_t timeStep = 0,
	       BodyProperty::Enum bodyProperty = BodyProperty::NONE);
    template<typename displaySameEdges>
    void writeFacesValues (
	const vector<boost::shared_ptr<Body> >& bodies, 
	BodyProperty::Enum bodyProperty);
    void initAddShader ();
    void initDisplayShader ();

private:
    /**
     * Stores the sum and count of values
     */
    boost::scoped_ptr<QGLFramebufferObject> m_new;
    /**
     * Stores the sum and count of the previous step.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_old;
    boost::scoped_ptr<QGLFramebufferObject> m_debug;
    struct
    {
	QGLShaderProgram shader;
	int vValueIndex;
	int oldTexUnitIndex;
    } m_add;

    struct
    {
	QGLShaderProgram shader;
	int minValueIndex;
	int maxValueIndex;
	int colorBarTexUnitIndex;
	int averageTexUnitIndex;
    } m_display;
};

#endif //__DISPLAY_FACE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
