/**
 * @file   AttributeAverages.cpp
 * @author Dan R. Lipsa
 * @date 11 Feb 2013
 *
 * Definitions for the view average
 */

#include "AttributeAverages.h"
#include "ForceAverage.h"
#include "ViewSettings.h"

// Private Functions
// ======================================================================




// Methods
// ======================================================================


AttributeAverages::AttributeAverages (
    ViewNumber::Enum viewNumber,
    boost::shared_ptr<Settings> settings, 
    boost::shared_ptr<const SimulationGroup> simulationGroup) :

    AverageInterface (viewNumber),
    m_forceAverage (new ForceAverage (viewNumber, settings, simulationGroup))
{
}

const Settings& AttributeAverages::GetSettings () const
{
    return m_scalarAverage->GetSettings ();
}


const ViewSettings& AttributeAverages::GetViewSettings () const
{
    return m_scalarAverage->GetViewSettings ();
}

const Simulation& AttributeAverages::GetSimulation () const
{
    return m_scalarAverage->GetSimulation ();
}

const Foam& AttributeAverages::GetFoam (ViewNumber::Enum viewNumber) const
{
    return m_scalarAverage->GetFoam (viewNumber);
}

void AttributeAverages::AverageInit ()
{
    m_scalarAverage->AverageInit ();
    m_forceAverage->AverageInit ();
    CALL_IF_NOT_NULL(m_velocityAverage,AverageInit) ();
    CALL_IF_NOT_NULL(m_deformationAverage,AverageInit) ();
    CALL_IF_NOT_NULL(m_t1KDE,AverageInit) ();
}

void AttributeAverages::AverageRelease ()
{
    m_scalarAverage->AverageRelease ();
    m_forceAverage->AverageRelease ();
    CALL_IF_NOT_NULL(m_velocityAverage,AverageRelease) ();
    CALL_IF_NOT_NULL(m_deformationAverage,AverageRelease) ();
    CALL_IF_NOT_NULL(m_t1KDE,AverageRelease) ();
}



void AttributeAverages::AverageStep (int direction, size_t timeWindow)
{
    const ViewSettings& vs = GetViewSettings ();
    switch (vs.GetViewType ())
    {
    case ViewType::AVERAGE:
	m_scalarAverage->AverageStep (direction, timeWindow);
	m_forceAverage->AverageStep (direction, timeWindow);
	CALL_IF_NOT_NULL(m_velocityAverage,AverageStep) (direction, timeWindow);
	CALL_IF_NOT_NULL(m_deformationAverage,AverageStep) (
            direction, timeWindow);
	break;
	
    case ViewType::T1_KDE:
	CALL_IF_NOT_NULL(m_velocityAverage,AverageStep) (direction, timeWindow);
	CALL_IF_NOT_NULL(m_t1KDE,AverageStep) (direction, timeWindow);
	break;
    default:
	break;
    }
}

void AttributeAverages::AverageRotateAndDisplay (
    StatisticsType::Enum displayType,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    const ViewSettings& vs = GetViewSettings ();
    switch (vs.GetViewType ())
    {
    case ViewType::AVERAGE:
	m_scalarAverage->AverageRotateAndDisplay (
	    displayType, rotationCenter, angleDegrees);
        m_forceAverage->AverageRotateAndDisplay (
	    displayType, rotationCenter, angleDegrees);
	if (vs.IsDeformationShown ())
	    CALL_IF_NOT_NULL(m_deformationAverage,AverageRotateAndDisplay) (
		displayType, rotationCenter, angleDegrees);
	break;
	
    case ViewType::T1_KDE:
	CALL_IF_NOT_NULL(m_t1KDE,AverageRotateAndDisplay) (
	    displayType, rotationCenter, angleDegrees);
	break;
    default:
	break;
    }
    if (vs.IsVelocityShown ())
        CALL_IF_NOT_NULL(m_velocityAverage,AverageRotateAndDisplay) (
            displayType, rotationCenter, angleDegrees);
}

