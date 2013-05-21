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
	const WidgetGl& widgetGl, AverageType::Enum type, QColor stepClearColor,
	FramebufferObjects& countFbos, size_t countIndex);
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
    AverageType::Enum GetAverageType () const
    {
	return m_averageType;
    }
    const WidgetGl& GetWidgetGl () const
    {
	return m_widgetGl;
    }
    G3D::Rect2D GetWindowCoord () const;

protected:
    struct FbosCountFbos
    {
        FbosCountFbos (
            boost::shared_ptr<QGLFramebufferObject> fbos,
            boost::shared_ptr<QGLFramebufferObject> countFbos,
            size_t countIndex) :
            m_fbos (fbos), m_countFbos (countFbos), m_countIndex (countIndex)
        {
        }
        boost::shared_ptr<QGLFramebufferObject> m_fbos;
        boost::shared_ptr<QGLFramebufferObject> m_countFbos;
        size_t m_countIndex;
    };

    virtual void addStep (size_t timeStep, size_t subStep);
    virtual void removeStep (size_t timeStep, size_t subStep);
    virtual void rotateAndDisplay (
	QwtDoubleInterval interval,
	StatisticsType::Enum displayType, FbosCountFbos fbo,
	ViewingVolumeOperation::Enum enclose,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const = 0;
    virtual void writeStepValues (ViewNumber::Enum viewNumber,
                                  size_t timeStep, size_t subStep);

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
    void save (FbosCountFbos fbo, 
	       const char* fileName, size_t timeStep, size_t subStep,
	       QwtDoubleInterval interval, StatisticsType::Enum displayType);
    void save (vtkSmartPointer<vtkFloatArray> data, 
               const G3D::Rect2D& windowCoord, 
               size_t components, float maxValue) const;


    FramebufferObjects m_fbos;
    FramebufferObjects& m_countFbos;
    size_t m_countIndex;

private:
    void clear ();


    void renderToStep (size_t timeStep, size_t subStep);
    void currentIsPreviousPlusStep ();
    void currentIsPreviousMinusStep ();
    void copyCurrentToPrevious ();
    void initFramebuffer (const boost::shared_ptr<QGLFramebufferObject>& fbo);
    
protected:
    CountType::Enum m_countType;

private:
    AverageType::Enum m_averageType;
    QColor m_stepClearColor;
    const WidgetGl& m_widgetGl;
};

#endif //__IMAGE_BASED_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
