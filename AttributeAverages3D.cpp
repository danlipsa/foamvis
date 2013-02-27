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
    
    boost::shared_ptr<RegularGridAverage> velocityAverage (
        new RegularGridAverage (viewNumber, settings, simulationGroup));
    velocityAverage->SetBodyAttribute (BodyAttribute::VELOCITY);
    m_velocityAverage = velocityAverage;

    boost::shared_ptr<RegularGridAverage> deformationAverage (
        new RegularGridAverage (viewNumber, settings, simulationGroup));
    deformationAverage->SetBodyAttribute (BodyAttribute::DEFORMATION);
    m_deformationAverage = deformationAverage;

    m_forceAverage.reset (
        new ForceAverage (viewNumber, settings, simulationGroup));
}
