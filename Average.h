/**
 * @file   Average.h
 * @author Dan R. Lipsa
 * @date  9 June 2010
 *
 * Interface for the Average class
 */

#ifndef __AVERAGE_H__
#define __AVERAGE_H__

#include "Enums.h"
#include "AverageInterface.h"

class Foam;
class Settings;
class Simulation;
class SimulationGroup;
class ViewSettings;


/**
 * Computes average along time for foam quantities. Includes average
 * for a time window behind the current time step for forward and
 * backward moving time.
 */
class Average : public AverageInterface
{
public:
    Average (ViewNumber::Enum viewNumber, 
	     const Settings& settings, const SimulationGroup& simulationGroup);
    const SimulationGroup& GetSimulationGroup () const
    {
	return m_simulationGroup;
    }
    const Settings& GetSettings () const
    {
	return m_settings;
    }
    const ViewSettings& GetViewSettings () const;   
    void AverageSetTimeWindow (size_t timeWindow)
    {
	m_timeWindow = timeWindow;
    }
    void AverageStep (int timeDifference);
    size_t GetTimeWindow () const
    {
	return m_timeWindow;
    }
    size_t GetCurrentTimeWindow () const
    {
	return m_currentTimeWindow;
    }
    virtual void AverageInit ();
    const Simulation& GetSimulation () const;
    const Foam& GetFoam (size_t timeStep) const;
    const Foam& GetFoam () const;
    size_t GetBodyAttribute () const;

protected:
    //@todo write and add/remove in one operations instead of two.
    virtual void addStep (size_t timeStep, size_t subStep) = 0;
    virtual void removeStep (size_t timeStep, size_t subStep) = 0;
    /**
     * A step is divident in stepSize subSteps
     */
    virtual size_t getStepSize (size_t timeStep) const
    {
	(void)timeStep;
	return 1;
    }

private:
    typedef void (Average::*Operation) (size_t timeStep, size_t subStep);
    void loopOperation (Operation op, size_t currentTime);



private:
    const Settings& m_settings;
    const SimulationGroup& m_simulationGroup;
    size_t m_currentTimeWindow;
    size_t m_timeWindow;
};

#endif //__AVERAGE_H__

// Local Variables:
// mode: c++
// End:
