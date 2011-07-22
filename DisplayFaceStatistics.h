/**
 * @file   DisplayFaceStatistics.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Interface for the DisplayFaceStatistics class
 */

#ifndef __DISPLAY_FACE_STATISTICS_H__
#define __DISPLAY_FACE_STATISTICS_H__

#include "DisplayElement.h"
#include "Enums.h"
#include "Average.h"

class Body;
class Foam;
class GLWidget;

/**
 * Shader that performs the following operation: current = previous + step
 * where current, previous and step are floating point textures
 * RGBA : sum, count, min, max
 */
class AddShaderProgram : public QGLShaderProgram
{
public:
    void Init ();
    void Bind ();
    GLint GetPreviousTexUnit ()
    {
	return 1;
    }
    GLint GetStepTexUnit ()
    {
	return 2;
    }
protected:
    int m_previousTexUnitIndex;
    int m_stepTexUnitIndex;
    boost::shared_ptr<QGLShader> m_fshader;
};


/**
 * Shader that performs the following operation: current = previous - step
 * where current, previous and step are floating point textures
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

/**
 * RGBA : sum, count, min, max
 */
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
 * It uses three framebuffer objects: step, previous, current.
 * Average is implemented by first calculating the sum and then dividing by
 * the number of elements in the sum. The sum is calculated in 3 steps:
 * 1. step = draw current foam
 * 2. current = previous + step
 * 3. previous = current
 *
 * The reason for this type of implementation is that OpenGL cannot
 * read and write to the same buffer in the same step.
 */
class DisplayFaceStatistics : public Average
{
public:
    DisplayFaceStatistics (const GLWidget& glWidget) :
	Average (glWidget)
    {
    }
    void Release ();
    void Display (ViewNumber::Enum viewNumber, StatisticsType::Enum displayType);
    void DisplayAndRotate (ViewNumber::Enum viewNumber,
			   StatisticsType::Enum displayType,
			   G3D::Vector2 rotationCenter, float angleDegrees);

public:
    static void InitShaders ();

protected:
    virtual void init (ViewNumber::Enum viewNumber);
    virtual void addStep (ViewNumber::Enum viewNumber, size_t timeStep);
    virtual void removeStep (ViewNumber::Enum viewNumber, size_t timeStep);

private:

    void clear (const G3D::Rect2D& viewRect);
    void writeFacesValues (
	ViewNumber::Enum view, const vector<boost::shared_ptr<Body> >& bodies);
    void display (const G3D::Rect2D& viewRect, 
		  GLfloat minValue, GLfloat maxValue,
		  StatisticsType::Enum displayType, QGLFramebufferObject& fbo);
    void displayAndRotate (const G3D::Rect2D& viewRect, 
			   GLfloat minValue, GLfloat maxValue,
			   StatisticsType::Enum displayType, 
			   QGLFramebufferObject& fbo,
			   G3D::Vector2 rotationCenter, float angleDegrees);

    void save (const G3D::Rect2D& viewRect,
	QGLFramebufferObject& fbo, const char* fileName, size_t timeStep,
	GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType);
    void renderToStep (ViewNumber::Enum view, size_t timeStep);
    void addStepToCurrent (const G3D::Rect2D& viewRect);
    void removeStepFromCurrent (const G3D::Rect2D& viewRect);
    void copyCurrentToPrevious ();
    void clearColorBufferMinMax (
	const G3D::Rect2D& viewRect,
	const boost::scoped_ptr<QGLFramebufferObject>& fbo);
    void glActiveTexture (GLenum texture) const;
    pair<double, double> getStatisticsMinMax (ViewNumber::Enum view) const;

private:
    /**
     * Stores (sum,count,min,max) up to and including the current step
     */
    boost::scoped_ptr<QGLFramebufferObject> m_current;
    /**
     * Stores (sum, count, min, max) up to and including the previous step.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_previous;
    /**
     * Stores (x, 1, x, x) for (sum, count, min, max) where x is the value for
     * one step. It stores (0, 0, maxFloat, -maxFloat) if there is no 
     * value for that pixel.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_step;
    /**
     * Used to save buffers as images.
     */
    boost::scoped_ptr<QGLFramebufferObject> m_debug;

    static AddShaderProgram m_addShaderProgram;
    static RemoveShaderProgram m_removeShaderProgram;
    static StoreShaderProgram m_storeShaderProgram;
    static DisplayShaderProgram m_displayShaderProgram;
    static InitShaderProgram m_initShaderProgram;
};

#endif //__DISPLAY_FACE_STATISTICS_H__

// Local Variables:
// mode: c++
// End:
