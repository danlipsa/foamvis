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


class AddShaderProgram : public QGLShaderProgram
{
public:
    void Init ();
    void Bind ();
    pair<GLint,GLenum> GetOldTexUnit ()
    {
	return pair<GLint,GLenum> (1, GL_TEXTURE1);
    }
    pair<GLint,GLenum> GetStepTexUnit ()
    {
	return pair<GLint,GLenum>(2, GL_TEXTURE2);
    }
private:
    int m_oldTexUnitIndex;
    int m_stepTexUnitIndex;    
};

class StoreShaderProgram : public QGLShaderProgram
{
public:
    void Init ();
    void Bind ();
    int GetVValueIndex () const
    {
	return m_vValueIndex;
    }
private:
    int m_vValueIndex;
};

class DisplayShaderProgram : public QGLShaderProgram
{
public:
    void Init ();
    void Bind (GLfloat minValue, GLfloat maxValue);

    // assume the colorbar is alreay bound on texture unit 0
    pair<GLint,GLenum> GetColorBarTexUnit ()
    {
	return pair<GLint,GLenum>(0,GL_TEXTURE0);
    }
    pair<GLint,GLenum> GetAverageTexUnit ()
    {
	return pair<GLint,GLenum>(1,GL_TEXTURE1);
    }

private:
    int m_minValueIndex;
    int m_maxValueIndex;
    int m_colorBarTexUnitIndex;
    int m_averageTexUnitIndex;    
};

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
    void Display (GLfloat minValue, GLfloat maxValue)
    {
	display (minValue, maxValue, *m_new);
    }
    void Calculate (BodyProperty::Enum bodyProperty);

private:
    void step (const Foam& foam, size_t timeStep,
	       BodyProperty::Enum bodyProperty);
    template<typename displaySameEdges>
    void writeFacesValues (
	const vector<boost::shared_ptr<Body> >& bodies, 
	BodyProperty::Enum bodyProperty);
    void display (GLfloat minValue, GLfloat maxValue, QGLFramebufferObject& fbo);
    void save (QGLFramebufferObject& fbo, string fileName, size_t timeStep);
    void renderToStep (const Foam& foam, BodyProperty::Enum bodyProperty);
    void addToNew ();
    void copyToOld ();
    static void clear (QGLFramebufferObject& fbo);


private:
    /**
     * Stores the sum and count of values
     */
    boost::scoped_ptr<QGLFramebufferObject> m_new;
    /**
     * Stores the sum and count of the previous step.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_old;
    /**
     * Stores the value for one step and 1 for the count 
     * (or [0, 0] for no value)
     */
    boost::scoped_ptr<QGLFramebufferObject> m_step;
    boost::scoped_ptr<QGLFramebufferObject> m_debug;
    AddShaderProgram m_addShaderProgram;
    StoreShaderProgram m_storeShaderProgram;
    DisplayShaderProgram m_displayShaderProgram;
};

#endif //__DISPLAY_FACE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
