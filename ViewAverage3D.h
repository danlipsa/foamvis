/**
 * @file   ViewAverage3D.h
 * @author Dan R. Lipsa
 * @date 7 February 2013
 *
 * Per view average calculation
 */
#ifndef __VIEW_AVERAGE_H__
#define __VIEW_AVERAGE_H__


#include "AverageInterface.h"
#include "Enums.h"

class ForceAverage;
class RegularGridAverage;
class Settings;
class SimulationGroup;
class ViewSettings;

class ViewAverage3D : public AverageInterface
{
public:
    ViewAverage3D (
        ViewNumber::Enum viewNumber,
        const Settings& settings, const SimulationGroup& simulationGroup);

    RegularGridAverage& GetScalarAverage () const
    {
	return *m_scalarAverage;
    }
    RegularGridAverage& GetVelocityAverage () const
    {
	return *m_velocityAverage;
    }    
    RegularGridAverage& GetDeformationAverage () const
    {
	return *m_deformationAverage;
    }

    ForceAverage& GetForceAverage () const
    {
	return *m_forceAverage;
    }
    virtual void AverageInit ();
    virtual void AverageStep (int direction, size_t timeWindow);
    virtual void AverageRotateAndDisplay (
	StatisticsType::Enum displayType = StatisticsType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    virtual void AverageRelease ();

private:
    boost::shared_ptr<RegularGridAverage> m_scalarAverage;
    boost::shared_ptr<RegularGridAverage> m_velocityAverage;
    boost::shared_ptr<RegularGridAverage> m_deformationAverage;
    boost::shared_ptr<ForceAverage> m_forceAverage;
};



#endif //__VIEW_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
