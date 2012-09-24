/**
 * @file   ImageBasedAverage.h
 * @author Dan R. Lipsa
 * @date  24 Oct. 2010
 *
 * Interface for the ImageBasedAverage class
 */

#ifndef __IMAGE_BASED_AVERAGE_H__
#define __IMAGE_BASED_AVERAGE_H__

#include "DisplayElement.h"
#include "Enums.h"
#include "Average.h"
#include "PropertySetter.h"

class Body;
class Foam;
class WidgetGl;
class ShaderProgram;
class AddShaderProgram;
class StoreShaderProgram;


struct FramebufferObjects
{
    /**
     * Stores values up to and including the current time step
     */
    boost::shared_ptr<QGLFramebufferObject> m_current;
    /**
     * Stores values up to and including the previous time step
     */
    boost::shared_ptr<QGLFramebufferObject> m_previous;
    /**
     * Stores values for the current time step
     */
    boost::shared_ptr<QGLFramebufferObject> m_step;
    /**
     * Used to save fbos as images.
     */
    boost::shared_ptr<QGLFramebufferObject> m_debug;
};


/**
 * Calculate the average of 2D foam over a time window.
 * It uses three framebuffer objects: step, previous, current.
 * Average is implemented by first calculating the sum and then dividing by
 * the number of elements in the sum. The sum is calculated in 3 steps:
 * 1. draw current foam using attribute values instead of colors into step
 * 2. current = previous + step
 * 3. previous = current
 *
 * The reason for this type of implementation is that OpenGL cannot
 * read and write to the same buffer in the same step.
 */
template<typename PropertySetter>
class ImageBasedAverage : public Average
{
public:
    ImageBasedAverage (
	ViewNumber::Enum viewNumber,
	const WidgetGl& widgetGl, string id, QColor stepClearColor,
	FramebufferObjects& countFbos);
    void AverageRelease ();
    void AverageRotateAndDisplay (	
	StatisticsType::Enum displayType = StatisticsType::AVERAGE,	
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    FramebufferObjects& GetFbos ()
    {
	return m_fbos;
    }
    virtual void AverageInit ();
    string GetId () const
    {
	return m_id;
    }    
    const WidgetGl& GetWidgetGl () const
    {
	return m_widgetGl;
    }

protected:
    typedef pair<boost::shared_ptr<QGLFramebufferObject>, 
		 boost::shared_ptr<QGLFramebufferObject> > 
	TensorScalarFbo;
    virtual void addStep (size_t timeStep, size_t subStep);
    virtual void removeStep (size_t timeStep, size_t subStep);
    virtual void rotateAndDisplay (
	GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, TensorScalarFbo fbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const = 0;
    virtual void writeStepValues (size_t timeStep, size_t subStep);

    void glActiveTexture (GLenum texture) const;
    QColor getStepClearColor ()
    {
	return m_stepClearColor;
    }
    vtkSmartPointer<vtkFloatArray> getData (
        boost::shared_ptr<QGLFramebufferObject> framebuffer,
        const G3D::Rect2D& windowCoord, GLenum format) const;

    static boost::shared_ptr<ShaderProgram> m_initShaderProgram;
    static boost::shared_ptr<StoreShaderProgram> m_storeShaderProgram;
    static boost::shared_ptr<AddShaderProgram> m_addShaderProgram;
    static boost::shared_ptr<AddShaderProgram> m_removeShaderProgram;

    FramebufferObjects& m_countFbos;
    FramebufferObjects m_fbos;

private:
    void clear ();

    void save (TensorScalarFbo fbo, 
	       const char* fileName, size_t timeStep, size_t subStep,
	       GLfloat minValue, 
	       GLfloat maxValue, StatisticsType::Enum displayType);
    void renderToStep (size_t timeStep, size_t subStep);
    void currentIsPreviousPlusStep ();
    void currentIsPreviousMinusStep ();
    void copyCurrentToPrevious ();
    void initFramebuffer (const boost::shared_ptr<QGLFramebufferObject>& fbo);
    
private:
    string m_id;
    QColor m_stepClearColor;
    const WidgetGl& m_widgetGl;
};

#endif //__IMAGE_BASED_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
