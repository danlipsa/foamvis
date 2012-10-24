/**
 * @file   ViewAverage.cpp
 * @author Dan R. Lipsa
 * @date 19 July 2012
 *
 * Definitions for the view average
 */

#include "ForceAverage.h"
#include "ScalarAverage.h"
#include "Simulation.h"
#include "T1sKDE.h"
#include "TensorAverage.h"
#include "VectorAverage.h"
#include "ViewAverage.h"
#include "WidgetGl.h"


// Private Functions
// ======================================================================




// Methods
// ======================================================================

ViewAverage::ViewAverage (ViewNumber::Enum viewNumber,
			  const WidgetGl& widgetGl,
			  const ViewSettings& viewSettings) :
    AverageInterface (viewNumber),
    m_scalarAverage (new ScalarAverage (viewNumber, widgetGl)),
    m_t1sKDE (new T1sKDE (viewNumber, widgetGl)),
    m_deformationAverage (
	new TensorAverage (viewNumber, widgetGl, m_scalarAverage->GetFbos ())),
    m_velocityAverage (
	new VectorAverage (viewNumber, widgetGl)),
    m_forceAverage (new ForceAverage (viewNumber, *widgetGl.GetSettings (),
				      widgetGl.GetSimulationGroup ())),
    m_viewSettings (viewSettings)
{
}

void ViewAverage::AverageInit ()
{
    GetScalarAverage ().AverageInit ();
    GetDeformationAverage ().AverageInit ();
    GetVelocityAverage ().AverageInit ();
    GetForceAverage ().AverageInit ();
    GetT1sKDE ().AverageInit ();
}

void ViewAverage::AverageRelease ()
{
    GetScalarAverage ().AverageRelease ();
    GetDeformationAverage ().AverageRelease ();
    GetVelocityAverage ().AverageRelease ();
    GetForceAverage ().AverageRelease ();
    GetT1sKDE ().AverageRelease ();
}


void ViewAverage::AverageSetTimeWindow (size_t timeSteps)
{
    GetScalarAverage ().AverageSetTimeWindow (timeSteps);
    GetDeformationAverage ().AverageSetTimeWindow (timeSteps);
    GetVelocityAverage ().AverageSetTimeWindow (timeSteps);
    GetForceAverage ().AverageSetTimeWindow (timeSteps);
}

void ViewAverage::AverageStep (int direction)
{
    switch (m_viewSettings.GetViewType ())
    {
    case ViewType::AVERAGE:
	GetScalarAverage ().AverageStep (direction);
	GetDeformationAverage ().AverageStep (direction);
	GetVelocityAverage ().AverageStep (direction);
	GetForceAverage ().AverageStep (direction);
	break;
	
    case ViewType::T1S_KDE:
	GetT1sKDE ().AverageStep (direction);
	GetVelocityAverage ().AverageStep (direction);
	break;
    default:
	break;
    }
}

void ViewAverage::AverageRotateAndDisplay (
    StatisticsType::Enum displayType,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    switch (m_viewSettings.GetViewType ())
    {
    case ViewType::AVERAGE:
	GetScalarAverage ().AverageRotateAndDisplay (
	    displayType, rotationCenter, angleDegrees);
	if (m_viewSettings.IsDeformationShown ())
	    GetDeformationAverage ().AverageRotateAndDisplay (
		displayType, rotationCenter, angleDegrees);
	break;
	
    case ViewType::T1S_KDE:
	GetT1sKDE ().AverageRotateAndDisplay (
	    displayType, rotationCenter, angleDegrees);
	break;
    default:
	break;
    }
    if (m_viewSettings.IsVelocityShown ())
    {
        VectorAverage& velocityAverage = GetVelocityAverage ();
        velocityAverage.SetGlyphShown (
            m_viewSettings.GetVelocityVis () == VectorVis::GLYPH);
        velocityAverage.AverageRotateAndDisplay (
            displayType, rotationCenter, angleDegrees);
    }
}


//@todo remove this and replace with AverageInit
//@todo overwrite AverageInit for T1sKDE
void ViewAverage::SetSimulation (const Simulation& simulation)
{
    AverageSetTimeWindow (simulation.GetTimeSteps ());
    GetT1sKDE ().AverageSetTimeWindow (simulation.GetT1sTimeSteps ());
}
