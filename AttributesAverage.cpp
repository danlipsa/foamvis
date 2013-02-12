/**
 * @file   AttributesAverage.cpp
 * @author Dan R. Lipsa
 * @date 11 Feb 2013
 *
 * Definitions for the view average
 */

#include "AttributesAverage.h"
#include "ForceAverage.h"

// Private Functions
// ======================================================================




// Methods
// ======================================================================


AttributesAverage::AttributesAverage (
    ViewNumber::Enum viewNumber,
    const Settings& settings, const SimulationGroup& simulationGroup) :
    AverageInterface (viewNumber),
    m_forceAverage (new ForceAverage (viewNumber, settings,
				      simulationGroup))
{
}

void AttributesAverage::AverageInit ()
{
    m_scalarAverage->AverageInit ();
    m_deformationAverage->AverageInit ();
    m_velocityAverage->AverageInit ();
    GetForceAverage ().AverageInit ();
    m_t1sKDE->AverageInit ();
}

void AttributesAverage::AverageRelease ()
{
    m_scalarAverage->AverageRelease ();
    m_deformationAverage->AverageRelease ();
    m_velocityAverage->AverageRelease ();
    GetForceAverage ().AverageRelease ();
    m_t1sKDE->AverageRelease ();
}



void AttributesAverage::AverageStep (int direction, size_t timeWindow)
{
    const ViewSettings& vs = GetSettings ()->GetViewSettings (GetViewNumber ());
    switch (vs.GetViewType ())
    {
    case ViewType::AVERAGE:
	m_scalarAverage.AverageStep (direction, timeWindow);
	m_deformationAverage.AverageStep (direction, timeWindow);
	m_velocityAverage.AverageStep (direction, timeWindow);
	GetForceAverage ().AverageStep (direction, timeWindow);
	break;
	
    case ViewType::T1S_KDE:
	m_t1sKDE.AverageStep (direction, timeWindow);
	m_velocityAverage.AverageStep (direction, timeWindow);
	break;
    default:
	break;
    }
}

void AttributesAverage::AverageRotateAndDisplay (
    StatisticsType::Enum displayType,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    const ViewSettings& vs = GetSettings ()->GetViewSettings (viewNumber);
    switch (vs.GetViewType ())
    {
    case ViewType::AVERAGE:
	m_scalarAverage.AverageRotateAndDisplay (
	    displayType, rotationCenter, angleDegrees);
	if (vs.IsDeformationShown ())
	    m_deformationAverage.AverageRotateAndDisplay (
		displayType, rotationCenter, angleDegrees);
	break;
	
    case ViewType::T1S_KDE:
	m_t1sKDE.AverageRotateAndDisplay (
	    displayType, rotationCenter, angleDegrees);
	break;
    default:
	break;
    }
    if (vs.IsVelocityShown ())
    {
        VectorAverage& velocityAverage = m_velocityAverage;
        velocityAverage.SetGlyphShown (
            vs.GetVelocityVis () == VectorVis::GLYPH);
        velocityAverage.AverageRotateAndDisplay (
            displayType, rotationCenter, angleDegrees);
    }
}

