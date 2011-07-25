/**
 * @file   ScalarAverage.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Interface for the ScalarAverage class
 */

#ifndef __SCALAR_AVERAGE_H__
#define __SCALAR_AVERAGE_H__

#include "DisplayElement.h"
#include "Enums.h"
#include "Average.h"

class Body;
class Foam;
class GLWidget;
class AddShaderProgram;
class RemoveShaderProgram;
class DisplayShaderProgram;
class InitShaderProgram;
class StoreShaderProgram;


/**
 * Calculate face average, min, max over a time window.
 * It uses three framebuffer objects: step, previous, current.
 * Average is implemented by first calculating the sum and then dividing by
 * the number of elements in the sum. The sum is calculated in 3 steps:
 * 1. step = draw current foam using attribute values instead of colors
 * 2. current = previous + step
 * 3. previous = current
 *
 * The reason for this type of implementation is that OpenGL cannot
 * read and write to the same buffer in the same step.
 */
class ScalarAverage : public Average
{
public:
    ScalarAverage (const GLWidget& glWidget) :
	Average (glWidget)
    {
    }
    void Release ();
    void Display (ViewNumber::Enum viewNumber, 
		  StatisticsType::Enum displayType);
    void DisplayAndRotate (
	ViewNumber::Enum viewNumber, StatisticsType::Enum displayType,
	G3D::Vector2 rotationCenter, float angleDegrees);
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
    void displayAndRotate (
	const G3D::Rect2D& viewRect, GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, QGLFramebufferObject& fbo,
	G3D::Vector2 rotationCenter, float angleDegrees);

    void save (const G3D::Rect2D& viewRect, QGLFramebufferObject& fbo, 
	       const char* fileName, size_t timeStep, GLfloat minValue, 
	       GLfloat maxValue, StatisticsType::Enum displayType);
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

    static InitShaderProgram m_initShaderProgram;
    static StoreShaderProgram m_storeShaderProgram;
    static AddShaderProgram m_addShaderProgram;
    static RemoveShaderProgram m_removeShaderProgram;
    static DisplayShaderProgram m_displayShaderProgram;
};

#endif //__SCALAR_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
