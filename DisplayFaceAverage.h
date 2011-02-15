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
#include "Enums.h"

class Body;
class Foam;
class GLWidget;

/**
 * Shader that performs the following operation: new = old + step
 * where new, old and step are floating point textures
 * RGBA : sum, count, min, max
 */
class ComposeShaderProgram : public QGLShaderProgram
{
public:
    void Init ();
    void Bind ();
    GLint GetOldTexUnit ()
    {
	return 1;
    }
    GLint GetStepTexUnit ()
    {
	return 2;
    }
private:
    int m_oldTexUnitIndex;
    int m_stepTexUnitIndex;
    boost::shared_ptr<QGLShader> m_fshader;
};

/**
 * Shader that stores a floating point value in a floating point texture:
 * RGBA: value, 1, value, value
 *
 */
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
    boost::shared_ptr<QGLShader> m_fshader;
    boost::shared_ptr<QGLShader> m_vshader;
};

class InitShaderProgram : public QGLShaderProgram
{
public:
    void Init ();
    void Bind ();
private:
    boost::shared_ptr<QGLShader> m_fshader;
};


class DisplayShaderProgram : public QGLShaderProgram
{
public:
    void Init ();
    void Bind (GLfloat minValue, GLfloat maxValue,
	       StatisticsType::Enum displayType);

    // assume the colorbar is alreay bound on texture unit 0
    GLint GetColorBarTexUnit ()
    {
	return 0;
    }
    GLint GetResultTexUnit ()
    {
	return 1;
    }

private:
    int m_displayTypeIndex;
    int m_minValueIndex;
    int m_maxValueIndex;
    int m_colorBarTexUnitIndex;
    int m_resultTexUnitIndex;
    boost::shared_ptr<QGLShader> m_fshader;
};


class DisplayFaceAverage : public DisplayElement
{
public:
    DisplayFaceAverage (const GLWidget& glWidget) :
	DisplayElement (glWidget)
    {
    }

    void Init (const QSize& size);
    void Clear ();
    void InitShaders ();
    void Release ();
    void Display (
	GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType = StatisticsType::AVERAGE)
    {
	display (minValue, maxValue, displayType, *m_new);
    }
    void Calculate (BodyProperty::Enum property,
		    GLfloat minValue, GLfloat maxValue);
    void StepDisplay ();
    void Step (size_t timeStep,
	       BodyProperty::Enum property,
	       GLfloat minValue, GLfloat maxValue);

private:
    template<typename displaySameEdges>
    void writeFacesValues (
	const vector<boost::shared_ptr<Body> >& bodies,
	BodyProperty::Enum property);
    void display (GLfloat minValue, GLfloat maxValue,
		  StatisticsType::Enum displayType, QGLFramebufferObject& fbo);
    void save (QGLFramebufferObject& fbo, string fileName, size_t timeStep,
	       GLfloat minValue, GLfloat maxValue,
	       StatisticsType::Enum displayType);
    void renderToStep (const Foam& foam, BodyProperty::Enum property);
    void addToNew ();
    void copyToOld ();
    static void clearZero (QGLFramebufferObject& fbo);
    void clearMinMax (QGLFramebufferObject& fbo);


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
    ComposeShaderProgram m_addShaderProgram;
    StoreShaderProgram m_storeShaderProgram;
    DisplayShaderProgram m_displayShaderProgram;
    InitShaderProgram m_initShaderProgram;
};

#endif //__DISPLAY_FACE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
