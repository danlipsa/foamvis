/**
 * @file   AttributeAverages3D.cpp
 * @author Dan R. Lipsa
 * @date 7 Feb 2013
 *
 * Definitions for the view average
 */

#include "ForceAverage.h"
#include "RegularGridAverage.h"
#include "Settings.h"
#include "AttributeAverages3D.h"
#include "ViewSettings.h"


AttributeAverages3D::AttributeAverages3D (
    ViewNumber::Enum viewNumber,
    boost::shared_ptr<Settings> settings, 
    boost::shared_ptr<const SimulationGroup> simulationGroup) :

    AttributeAverages (viewNumber, settings, simulationGroup)
{
    m_scalarAverage.reset (
        new RegularGridAverage (viewNumber, settings, simulationGroup));
    
    boost::shared_ptr<RegularGridAverage> average (
        new RegularGridAverage (viewNumber, settings, simulationGroup));
    average->SetBodyAttribute (BodyAttribute::VELOCITY);
    m_velocityAverage = average;


    average.reset (
        new RegularGridAverage (viewNumber, settings, simulationGroup));
    average->SetBodyAttribute (OtherScalar::T1_KDE);
    m_t1KDE = average;

/*
    boost::shared_ptr<RegularGridAverage> deformationAverage (
        new RegularGridAverage (viewNumber, settings, simulationGroup));
    deformationAverage->SetBodyAttribute (BodyAttribute::DEFORMATION);
    m_deformationAverage = deformationAverage;
*/

    m_forceAverage.reset (
        new ForceAverage (viewNumber, settings, simulationGroup));
}

void AttributeAverages3D::ComputeAverage ()
{
    const ViewSettings& vs = GetViewSettings ();
    switch (vs.GetViewType ())
    {
    case ViewType::AVERAGE:
	GetScalarAverage ()->ComputeAverage ();
        CALL_IF_NOT_NULL(GetVelocityAverage (),ComputeAverage) ();
        CALL_IF_NOT_NULL(GetDeformationAverage (),ComputeAverage) ();
	break;
	
    case ViewType::T1_KDE:
	CALL_IF_NOT_NULL(GetT1KDE (),ComputeAverage) ();
	break;

    default:
	break;
    }

}

boost::shared_ptr<RegularGridAverage> 
AttributeAverages3D::GetBodyOrOtherScalarAverage ()
{
    const ViewSettings& vs = GetViewSettings ();
    size_t bodyOrOtherScalar = vs.GetBodyOrOtherScalar ();
    return (bodyOrOtherScalar == OtherScalar::T1_KDE) ? 
        GetT1KDE () : GetScalarAverage ();
}
