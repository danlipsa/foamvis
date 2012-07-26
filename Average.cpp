/**
 * @file   Average.h
 * @author Dan R. Lipsa
 * @date  9 June 2010
 *
 * Implementation for the Average class 
 *
 */

#include "Average.h"
#include "Settings.h"
#include "Simulation.h"
#include "OpenGLUtils.h"
#include "ViewSettings.h"

void Average::AverageInit (ViewNumber::Enum viewNumber)
{
    AverageInterface::AverageInit (viewNumber);
    m_currentTimeWindow = 0;
}

void Average::loopOperation (Operation op, size_t currentTime)
{
    size_t stepSize = getStepSize (currentTime);
    for (size_t i = 0; i < stepSize; ++i)
	(this->*op) (currentTime, i);
}

void Average::AverageStep (int timeDifference)
{
    if (abs (timeDifference) > 1)
    {
	AverageInitStep (GetViewNumber ());
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

const Simulation& Average::GetSimulation () const
{
    return
	GetSimulationGroup ().GetSimulation (
	    GetSettings ().GetViewSettings (
		GetViewNumber ()).GetSimulationIndex ());
}

const Foam& Average::GetFoam (size_t timeStep) const
{
    return GetSimulation ().GetFoam (timeStep);
}
