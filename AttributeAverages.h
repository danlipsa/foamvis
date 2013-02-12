/**
 * @file   AttributeAverages.h
 * @author Dan R. Lipsa
 * @date 11 February 2013
 *
 * Per view average calculation
 */
#ifndef __ATTRIBUTE_AVERAGES_H__
#define __ATTRIBUTE_AVERAGES_H__

#include "AverageInterface.h"
#include "Enums.h"

class Average;
class Foam;
class ForceAverage;
class Settings;
class Simulation;
class SimulationGroup;
class ViewSettings;

class AttributeAverages : public AverageInterface
{
public:
    AttributeAverages (
        ViewNumber::Enum viewNumber,
        const Settings& settings, const SimulationGroup& simulationGroup);

    const ForceAverage& GetForceAverage () const
    {
	return *m_forceAverage;
    }
    const Settings& GetSettings () const;
    const ViewSettings& GetViewSettings () const;
    const Foam& GetFoam () const;
    const Simulation& GetSimulation () const;

    virtual void AverageInit ();
    virtual void AverageStep (int direction, size_t timeWindow);
    virtual void AverageRotateAndDisplay (
	StatisticsType::Enum displayType = StatisticsType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    virtual void AverageRelease ();


protected:
    boost::shared_ptr<Average> m_scalarAverage;
    boost::shared_ptr<ForceAverage> m_forceAverage;
    boost::shared_ptr<Average> m_velocityAverage;
    boost::shared_ptr<Average> m_deformationAverage;
    boost::shared_ptr<Average> m_t1sKDE;
};

#endif //__ATTRIBUTE_AVERAGES_H__

// Local Variables:
// mode: c++
// End:
