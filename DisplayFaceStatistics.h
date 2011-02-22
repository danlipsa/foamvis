/**
 * @file   DisplayFaceStatistics.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Interface for the DisplayFaceStatistics class
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
class AddShaderProgram : public QGLShaderProgram
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
protected:
    int m_oldTexUnitIndex;
    int m_stepTexUnitIndex;
    boost::shared_ptr<QGLShader> m_fshader;
};


/**
 * Shader that performs the following operation: new = old - step
 * where new, old and step are floating point textures
 * RGBA : sum, count, min, max. It leaves min and max values unchanged.
 */
class RemoveShaderProgram : public AddShaderProgram
{
public:
    void Init ();
    void Bind ();
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


/**
 * Calculate face average, min, max over all time steps.
 * It uses three framebuffer objects: step, old, new.
 * Average is implemented by first calculating the sum and then dividing by
 * the number of elements in the sum. The sum is calculated in 3 steps:
 * 1. step = draw current foam
 * 2. new = step + old
 * 3. old = new
 *
 * The reason for this type of implementation is that OpenGL cannot
 * read and write to the same buffer in the same step.
 */
class DisplayFaceStatistics : public DisplayElement
{
public:
    DisplayFaceStatistics (const GLWidget& glWidget) :
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
	if (m_new.get () != 0)
	    display (minValue, maxValue, displayType, *m_new);
    }
    void InitStepDisplay ();
    void StepDisplay ();
    void Step (size_t timeStep,
	       BodyProperty::Enum property,
	       GLfloat minValue, GLfloat maxValue);
    void SetHistoryCount (size_t historyCount)
    {
	m_historyCount = historyCount;
    }

private:
    template<typename displaySameEdges>
    void writeFacesValues (
	const vector<boost::shared_ptr<Body> >& bodies,
	BodyProperty::Enum property);
    void display (GLfloat minValue, GLfloat maxValue,
		  StatisticsType::Enum displayType, QGLFramebufferObject& fbo);
    void save (QGLFramebufferObject& fbo, const char* fileName, size_t timeStep,
	       GLfloat minValue, GLfloat maxValue,
	       StatisticsType::Enum displayType);
    void renderToStep (size_t timeStep, BodyProperty::Enum property);
    void addStepToNew ();
    void removeStepFromNew ();
    void copyNewToOld ();
    static void clearZero (
	const boost::scoped_ptr<QGLFramebufferObject>& fbo);
    void clearMinMax (
	const boost::scoped_ptr<QGLFramebufferObject>& fbo);


private:
    /**
     * Stores (sum,count,min,max) up too and including the current step
     */
    boost::scoped_ptr<QGLFramebufferObject> m_new;
    /**
     * Stores (sum, count, min, max) up to and including the previous step.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_old;
    /**
     * Stores (x, 1, x, x) for (sum, count, min, max) where x is the value for
     * one step. It stores (0, 0, 0, 0) if there is no value for that pixel.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_step;
    boost::scoped_ptr<QGLFramebufferObject> m_debug;

    size_t m_currentHistoryCount;
    size_t m_historyCount;

    AddShaderProgram m_addShaderProgram;
    RemoveShaderProgram m_removeShaderProgram;
    StoreShaderProgram m_storeShaderProgram;
    DisplayShaderProgram m_displayShaderProgram;
    InitShaderProgram m_initShaderProgram;
};

#endif //__DISPLAY_FACE_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
