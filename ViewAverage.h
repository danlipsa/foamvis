/**
 * @file   ViewAverage.h
 * @author Dan R. Lipsa
 * @date 19 July 2012
 *
 * Per view average calculation
 */
#ifndef __VIEW_AVERAGE_H__
#define __VIEW_AVERAGE_H__


#include "AverageInterface.h"
#include "Enums.h"

class ForceAverage;
class ScalarAverage;
class T1sPDE;
class TensorAverage;
class WidgetGl;
class VectorAverage;
class ViewSettings;

class ViewAverage : public AverageInterface
{
public:
    ViewAverage (const WidgetGl& widgetGl, const ViewSettings& viewSettings);

    ScalarAverage& GetScalarAverage () const
    {
	return *m_scalarAverage;
    }
    
    T1sPDE& GetT1sPDE () const
    {
	return *m_t1sPDE;
    }

    TensorAverage& GetDeformationAverage () const
    {
	return *m_deformationAverage;
    }
    VectorAverage& GetVelocityAverage () const
    {
	return *m_velocityAverage;
    }

    ForceAverage& GetForceAverage () const
    {
	return *m_forceAverage;
    }
    void SetSimulation (const Simulation& simulation);
    virtual void AverageInit (ViewNumber::Enum viewNumber);
    virtual void AverageSetTimeWindow (size_t timeSteps);
    virtual void AverageStep (int direction);
    virtual void AverageRotateAndDisplay (
	ComputationType::Enum displayType = ComputationType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    virtual void AverageRelease ();

private:
    boost::shared_ptr<ScalarAverage> m_scalarAverage;
    boost::shared_ptr<T1sPDE> m_t1sPDE;
    boost::shared_ptr<TensorAverage> m_deformationAverage;
    boost::shared_ptr<VectorAverage> m_velocityAverage;
    boost::shared_ptr<ForceAverage> m_forceAverage;
    const ViewSettings& m_viewSettings;
};



#endif //__VIEW_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
