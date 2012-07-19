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
#include "T1sPDE.h"
#include "TensorAverage.h"
#include "VectorAverage.h"
#include "ViewAverage.h"


// Private Functions
// ======================================================================




// Methods
// ======================================================================

ViewAverage::ViewAverage (const WidgetGl& widgetGl,
			  const ViewSettings& viewSettings) :
    m_scalarAverage (new ScalarAverage (widgetGl)),
    m_t1sPDE (new T1sPDE (widgetGl)),
    m_deformationAverage (
	new TensorAverage (widgetGl, m_scalarAverage->GetFbos ())),
    m_velocityAverage (
	new VectorAverage (widgetGl, m_scalarAverage->GetFbos ())),
    m_forceAverage (new ForceAverage (widgetGl)),
    m_viewSettings (viewSettings)
{
}

void ViewAverage::AverageInit (ViewNumber::Enum viewNumber)
{
    GetScalarAverage ().AverageInit (viewNumber);
    GetDeformationAverage ().AverageInit (viewNumber);
    GetVelocityAverage ().AverageInit (viewNumber);
    GetForceAverage ().AverageInit (viewNumber);
    GetT1sPDE ().AverageInit (viewNumber);
}

void ViewAverage::AverageSetTimeWindow (size_t timeSteps)
{
    GetScalarAverage ().AverageSetTimeWindow (timeSteps);
    GetDeformationAverage ().AverageSetTimeWindow (timeSteps);
    GetVelocityAverage ().AverageSetTimeWindow (timeSteps);
    GetForceAverage ().AverageSetTimeWindow (timeSteps);
}

void ViewAverage::AverageStep (ViewNumber::Enum viewNumber, int direction)
{
    switch (m_viewSettings.GetViewType ())
    {
    case ViewType::AVERAGE:
	GetScalarAverage ().AverageStep (viewNumber, direction);
	GetDeformationAverage ().AverageStep (viewNumber, direction);
	GetVelocityAverage ().AverageStep (viewNumber, direction);
	GetForceAverage ().AverageStep (viewNumber, direction);
	break;
	
    case ViewType::T1S_PDE:
	GetT1sPDE ().AverageStep (viewNumber, direction);
	break;
    default:
	break;
    }
}

void ViewAverage::AverageRotateAndDisplay (
    ViewNumber::Enum viewNumber, ComputationType::Enum displayType,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    switch (m_viewSettings.GetViewType ())
    {
    case ViewType::AVERAGE:
	GetScalarAverage ().AverageRotateAndDisplay (
	    viewNumber, displayType, rotationCenter, angleDegrees);
	if (m_viewSettings.IsDeformationShown ())
	    GetDeformationAverage ().AverageRotateAndDisplay (
		viewNumber, displayType, rotationCenter, angleDegrees);
	if (m_viewSettings.IsVelocityShown ())
	    GetVelocityAverage ().AverageRotateAndDisplay (
		viewNumber, displayType, rotationCenter, angleDegrees);	    
	break;
	
    case ViewType::T1S_PDE:
	GetT1sPDE ().AverageRotateAndDisplay (
	    viewNumber, displayType, rotationCenter, angleDegrees);
	break;
    default:
	break;
    }
}

void ViewAverage::AverageRelease ()
{
    GetScalarAverage ().AverageRelease ();
    GetDeformationAverage ().AverageRelease ();
    GetVelocityAverage ().AverageRelease ();
    GetT1sPDE ().AverageRelease ();
}

void ViewAverage::SetSimulation (const Simulation& simulation)
{
    AverageSetTimeWindow (simulation.GetTimeSteps ());
    GetT1sPDE ().AverageSetTimeWindow (simulation.GetT1sTimeSteps ());
}
