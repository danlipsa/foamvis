/**
 * @file   AttributeAverages.h
 * @author Dan R. Lipsa
 * @date 11 February 2013
 * @brief Computes the average for several attributes in a view.
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

/**
 * @brief Computes the average for several attributes in a view.
 */
class AttributeAverages : public AverageInterface
{
public:
    AttributeAverages (
        ViewNumber::Enum viewNumber,
        boost::shared_ptr<Settings> settings, 
        boost::shared_ptr<const SimulationGroup> simulationGroup);

    boost::shared_ptr<ForceAverage> GetForceAverage ()
    {
	return m_forceAverage;
    }
    const Settings& GetSettings () const;
    const ViewSettings& GetViewSettings () const;
    const Foam& GetFoam (ViewNumber::Enum viewNumber) const;
    const Simulation& GetSimulation () const;
    ViewType::Enum GetInitViewType () const
    {
        return m_initViewType;
    }

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
    boost::shared_ptr<Average> m_t1KDE;
    ViewType::Enum m_initViewType;
};

#endif //__ATTRIBUTE_AVERAGES_H__

// Local Variables:
// mode: c++
// End:
