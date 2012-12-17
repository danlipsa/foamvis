/**
 * @file   Average.h
 * @author Dan R. Lipsa
 * @date  9 June 2010
 *
 * Implementation for the Average class 
 *
 */

#include "Average.h"
#include "Debug.h"
#include "DebugStream.h"
#include "Settings.h"
#include "Simulation.h"
#include "OpenGLUtils.h"
#include "ViewSettings.h"


Average::Average (
    ViewNumber::Enum viewNumber, 
    const Settings& settings, const SimulationGroup& simulationGroup) :
    AverageInterface (viewNumber),
    
    m_settings (settings),
    m_simulationGroup (simulationGroup),
    m_currentTimeWindow (0)
{
}


void Average::AverageInit ()
{
    m_currentTimeWindow = 0;
}

void Average::forAllSubsteps (Operation op, size_t currentTime)
{
    size_t stepSize = getStepSize (currentTime);
    for (size_t i = 0; i < stepSize; ++i)
	(this->*op) (currentTime, i);
}

void Average::executeOperation (
    size_t currentTime, Operation op, TimeOperation timeOp, 
    bool atEnd, size_t timeWindow)
{
    if (atEnd)
    {
        do
        {
            forAllSubsteps (op, currentTime);
            currentTime = timeOp (currentTime, 1);
            --m_currentTimeWindow;
        }
        while (m_currentTimeWindow >= timeWindow);
        ++m_currentTimeWindow;
    }
    else
        forAllSubsteps (op, currentTime);
}

void Average::AverageStep (int timeDifference, size_t timeWindow)
{
    if (timeDifference == 0)
	return;
    if (abs (timeDifference) > 1)
    {
	AverageInitStep (timeWindow);
	return;
    }
    Operation firstOp, secondOp;
    size_t currentTime = m_settings.GetViewTime (GetViewNumber ());
    if (timeDifference < 0)
    {
	++currentTime;
	firstOp = &Average::removeStep;
	secondOp = &Average::addStep;
    }
    else
    {
	firstOp = &Average::addStep;
	secondOp = &Average::removeStep;
    }
    executeOperation (
        currentTime, firstOp, std::minus<float> (), 
        timeDifference < 0, timeWindow);
    if (m_currentTimeWindow >= timeWindow && currentTime >= timeWindow)
        executeOperation (currentTime - m_currentTimeWindow,
                          secondOp, std::plus<float> (), 
                          timeDifference > 0, timeWindow);
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
	GetSimulationGroup ().GetSimulation (GetSettings (), GetViewNumber ());
}

const Foam& Average::GetFoam (size_t timeStep) const
{
    return GetSimulation ().GetFoam (timeStep);
}

const Foam& Average::GetFoam () const
{
    size_t currentTime = GetViewSettings ().GetTime ();
    return GetSimulation ().GetFoam (currentTime);
}


size_t Average::GetBodyAttribute () const
{
    return GetViewSettings ().GetBodyOrFaceScalar ();
}
