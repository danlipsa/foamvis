/**
 * @file   Average.h
 * @author Dan R. Lipsa
 * @date  9 June 2010
 * @brief Computes a time-average of a foam attribute. 
 * @ingroup average
 */

#ifndef __AVERAGE_H__
#define __AVERAGE_H__

#include "AverageInterface.h"
#include "Base.h"

class Foam;
class Settings;
class Simulation;
class SimulationGroup;
class ViewSettings;


/**
 * @brief Computes a time-average of a foam attribute.
 * 
 * Includes average for a time window behind the current time step for
 * forward and backward moving time.
 */
class Average : public AverageInterface, public Base
{
public:
    Average (ViewNumber::Enum viewNumber, 
	     boost::shared_ptr<Settings> settings, 
             boost::shared_ptr<const SimulationGroup> simulationGroup);
    void AverageStep (int timeDifference, size_t timeWindow);
    size_t GetCurrentTimeWindow () const
    {
	return m_currentTimeWindow;
    }
    virtual void AverageInit ();

    G3D::Vector3 GetTranslation (size_t timeStep) const;
    G3D::Vector3 GetTranslation () const;

    virtual ViewNumber::Enum GetViewNumber () const;


protected:
    /**
     * @todo Write and add/remove in one operations instead of two.
     */
    virtual void addStep (size_t timeStep, size_t subStep) = 0;
    virtual void removeStep (size_t timeStep, size_t subStep) = 0;
    /**
     * A step is divided in stepSize subSteps. This is used for T1s where
     * there are several t1s for one time step.
     */
    virtual size_t getStepSize (size_t timeStep) const
    {
	(void)timeStep;
	return 1;
    }

private:
    typedef void (Average::*Operation) (size_t timeStep, size_t subStep);
    typedef boost::function <float (float, float)> TimeOperation;
    void forAllSubsteps (Operation op, size_t currentTime);
    void executeOperation (
        size_t currentTime, Operation op, TimeOperation timeOp, 
        bool atEnd, size_t timeWindow);

private:
    size_t m_currentTimeWindow;
};

#endif //__AVERAGE_H__

// Local Variables:
// mode: c++
// End:
