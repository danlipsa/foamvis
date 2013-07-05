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
#include "DerivedData.h"
#include "ObjectPosition.h"
#include "OpenGLUtils.h"
#include "Settings.h"
#include "Simulation.h"
#include "ViewSettings.h"


Average::Average (
    ViewNumber::Enum viewNumber, boost::shared_ptr<Settings> settings, 
    boost::shared_ptr<const SimulationGroup> simulationGroup, 
    boost::shared_ptr<DerivedData>* dd) :
    AverageInterface (viewNumber), Base (settings, simulationGroup, dd),
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
    size_t currentTime = GetSettings ().GetViewTime (GetViewNumber ());
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



G3D::Vector3 Average::GetTranslation (size_t timeStep) const
{
    const Simulation& simulation = GetSimulation ();
    G3D::AABox bb = simulation.GetBoundingBox ();
    G3D::Vector3 center = bb.center ();
    const ObjectPosition current = 
        GetObjectPositions ()->GetAverageAroundPosition (timeStep);
    G3D::Vector3 t = center - current.m_rotationCenter;
    return t;
}

G3D::Vector3 Average::GetTranslation () const
{
    const ViewSettings& vs = GetViewSettings ();
    return GetTranslation (vs.GetTime ());
}

ViewNumber::Enum Average::GetViewNumber () const
{
    return AverageInterface::GetViewNumber ();
}
