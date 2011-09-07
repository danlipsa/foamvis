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
class GLWidget;
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
 * Calculate an average a time window.
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
template<typename PropertySetter>
class ImageBasedAverage : public Average
{
public:
    ImageBasedAverage (
	const GLWidget& glWidget, string id, 
	FramebufferObjects& scalarAverageFbos) :
	Average (glWidget), m_scalarAverageFbos (scalarAverageFbos), m_id (id)
    {
    }
    void AverageRelease ();
    void AverageRotateAndDisplay (	
	ViewNumber::Enum viewNumber, StatisticsType::Enum displayType,
	
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    FramebufferObjects& GetFbos ()
    {
	return m_fbos;
    }
    virtual void AverageInit (ViewNumber::Enum viewNumber);
    string GetId () const
    {
	return m_id;
    }

protected:
    typedef pair<boost::shared_ptr<QGLFramebufferObject>, 
		 boost::shared_ptr<QGLFramebufferObject> > 
	FramebufferObjectPair;
    virtual void addStep (ViewNumber::Enum viewNumber, size_t timeStep);
    virtual void removeStep (ViewNumber::Enum viewNumber, size_t timeStep);
    virtual void rotateAndDisplay (
	ViewNumber::Enum viewNumber,
	GLfloat minValue, GLfloat maxValue,
	StatisticsType::Enum displayType, FramebufferObjectPair fbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const = 0;
    virtual void writeStepValues (ViewNumber::Enum view, size_t timeStep);

    void glActiveTexture (GLenum texture) const;


    static boost::shared_ptr<ShaderProgram> m_initShaderProgram;
    static boost::shared_ptr<StoreShaderProgram> m_storeShaderProgram;
    static boost::shared_ptr<AddShaderProgram> m_addShaderProgram;
    static boost::shared_ptr<AddShaderProgram> m_removeShaderProgram;

    FramebufferObjects& m_scalarAverageFbos;
    FramebufferObjects m_fbos;

private:
    void clear (ViewNumber::Enum viewNumber);

    void save (ViewNumber::Enum viewNumber, FramebufferObjectPair fbo, 
	       const char* fileName, size_t timeStep, GLfloat minValue, 
	       GLfloat maxValue, StatisticsType::Enum displayType);
    void renderToStep (ViewNumber::Enum view, size_t timeStep);
    void currentIsPreviousPlusStep (ViewNumber::Enum viewNumber);
    void currentIsPreviousMinusStep (ViewNumber::Enum viewNumber);
    void copyCurrentToPrevious ();
    void initFramebuffer (
	ViewNumber::Enum viewNumber,
	const boost::shared_ptr<QGLFramebufferObject>& fbo);
    pair<double, double> getStatisticsMinMax (ViewNumber::Enum view) const;

private:
    string m_id;
};

#endif //__IMAGE_BASED_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
