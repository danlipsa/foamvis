/**
 * @file   ObjectPosition.cpp
 * @author Dan R. Lipsa
 * @date 7 July 2013
 *
 * Definitions for the ObjectPostion
 */

#include "ObjectPosition.h"
#include "Body.h"
#include "Foam.h"
#include "OpenGLUtils.h"
#include "Simulation.h"

void ObjectPositions::SetAverageAroundPositions (
    const Simulation& simulation)
{
    m_averageAroundPositions.resize (simulation.GetTimeSteps ());
    for (size_t i = 0; i < m_averageAroundPositions.size (); ++i)
	m_averageAroundPositions[i] = 
	    simulation.GetFoam (i).GetDmpObjectPosition ();
}

void ObjectPositions::SetAverageAroundPositions (
    const Simulation& simulation, size_t bodyId)
{
    m_averageAroundPositions.resize (simulation.GetTimeSteps ());
    for (size_t i = 0; i < m_averageAroundPositions.size (); ++i)
    {
	ObjectPosition& objectPosition = m_averageAroundPositions[i];
	const Foam& foam = simulation.GetFoam (i);
	objectPosition.m_angleRadians = 0;
	objectPosition.m_rotationCenter = 
	    (*foam.FindBody (bodyId))->GetCenter ();
    }
}

void ObjectPositions::SetAverageAroundPositions (
    const Simulation& simulation,
    size_t bodyId, size_t secondBodyId)
{
    G3D::Vector2 beginAxis = 
	simulation.GetFoam (0).GetAverageAroundAxis (bodyId, secondBodyId);
    // the angle for i = 0 is already set to 0. Trying to calculate will results 
    // in acosValue slightly greater than 1, a Nan and then an error 
    // in gluUnProject.
    for (size_t i = 1; i < m_averageAroundPositions.size (); ++i)
    {
	ObjectPosition& objectPosition = m_averageAroundPositions[i];
	const Foam& foam = simulation.GetFoam (i);

	G3D::Vector2 currentAxis = 
	    foam.GetAverageAroundAxis (bodyId, secondBodyId);
	float acosValue = currentAxis.direction ().dot (beginAxis.direction ());
	float angleRadians = acos (acosValue);
	objectPosition.m_angleRadians = - angleRadians;
    }
}
