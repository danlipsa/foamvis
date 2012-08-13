/**
 * @file   Average.h
 * @author Dan R. Lipsa
 * @date  9 June 2010
 *
 * Implementation for the Average class 
 *
 */

#include "Average.h"
#include "DebugStream.h"
#include "Settings.h"
#include "Simulation.h"
#include "OpenGLUtils.h"
#include "ViewSettings.h"

//#define __LOG__(code) code
#define __LOG__(code)


Average::Average (
    ViewNumber::Enum viewNumber, 
    const Settings& settings, const SimulationGroup& simulationGroup) :
    AverageInterface (viewNumber),
    
    m_settings (settings),
    m_simulationGroup (simulationGroup),
    m_currentTimeWindow (0),
    m_timeWindow (0)
{
}


void Average::AverageInit ()
{
    m_currentTimeWindow = 0;
    m_timeWindow = GetSimulation ().GetFoams ().size ();
}

void Average::loopOperation (Operation op, size_t currentTime)
{
    size_t stepSize = getStepSize (currentTime);
    for (size_t i = 0; i < stepSize; ++i)
	(this->*op) (currentTime, i);
}

void Average::AverageStep (int timeDifference)
{
    __LOG__ (cdbg << "AverageStep: " << timeDifference << endl;)
    if (timeDifference == 0)
	return;
    if (abs (timeDifference) > 1)
    {
	AverageInitStep ();
	return;
    }
    Operation first, second;
    size_t currentTime = m_settings.GetCurrentTime (GetViewNumber ());
    if (timeDifference < 0)
    {
	++currentTime;
	first = &Average::removeStep;
	second = &Average::addStep;
    }
    else
    {
	first = &Average::addStep;
	second = &Average::removeStep;
    }
    loopOperation (first, currentTime);
    if (m_currentTimeWindow >= m_timeWindow && currentTime >= m_timeWindow)
    {
	currentTime -= m_timeWindow;
	loopOperation (second, currentTime);
    }
    else
	m_currentTimeWindow += timeDifference;
    WarnOnOpenGLError ("AverageStep");
}

const ViewSettings& Average::GetViewSettings () const
{
    return GetSettings ().GetViewSettings (GetViewNumber ());
}


const Simulation& Average::GetSimulation () const
{
    return
	GetSimulationGroup ().GetSimulation (
	    GetViewSettings ().GetSimulationIndex ());
}

const Foam& Average::GetFoam (size_t timeStep) const
{
    return GetSimulation ().GetFoam (timeStep);
}

const Foam& Average::GetFoam () const
{
    size_t currentTime = GetViewSettings ().GetCurrentTime ();
    return GetSimulation ().GetFoam (currentTime);
}


size_t Average::GetBodyAttribute () const
{
    return GetViewSettings ().GetBodyOrFaceScalar ();
}
