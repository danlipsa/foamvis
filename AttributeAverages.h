/**
 * @file   AttributesAverage.h
 * @author Dan R. Lipsa
 * @date 11 February 2013
 *
 * Per view average calculation
 */
#ifndef __ATTRIBUTES_AVERAGE_H__
#define __ATTRIBUTES_AVERAGE_H__

#include "AverageInterface.h"
#include "Enums.h"

class Average;
class ForceAverage;
class Settings;
class SimulationGroup;
class ViewSettings;

class AttributesAverage : public AverageInterface
{
public:
    AttributesAverage (
        ViewNumber::Enum viewNumber,
        const Settings& settings, const SimulationGroup& simulationGroup);

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
    const Settings& GetSettings () const;
    const ViewSettings& GetViewSettings () const;

protected:
    boost::shared_ptr<Average> m_scalarAverage;
    boost::shared_ptr<Average> m_t1sKDE;
    boost::shared_ptr<Average> m_deformationAverage;
    boost::shared_ptr<Average> m_velocityAverage;
    boost::shared_ptr<ForceAverage> m_forceAverage;
};

#endif //__ATTRIBUTES_AVERAGE_H__

// Local Variables:
// mode: c++
// End:
