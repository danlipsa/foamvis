/**
 * @file   AttributesAverage3D.cpp
 * @author Dan R. Lipsa
 * @date 7 Feb 2013
 *
 * Definitions for the view average
 */

#include "ForceAverage.h"
#include "RegularGridAverage.h"
#include "Settings.h"
#include "AttributesAverage3D.h"
#include "ViewSettings.h"


AttributesAverage3D::AttributesAverage3D (
    ViewNumber::Enum viewNumber,
    const Settings& settings, const SimulationGroup& simulationGroup) :

    AttributesAverage (viewNumber, settings, simulationGroup)
{
    m_scalarAverage.reset (
        new RegularGridAverage (
            settings.GetViewSettings (viewNumber).GetBodyOrFaceScalar (),
            viewNumber, settings, simulationGroup));
    m_velocityAverage.reset (
        new RegularGridAverage (
            BodyAttribute::VELOCITY, viewNumber, settings, simulationGroup));
    m_deformationAverage.reset (
        new RegularGridAverage (
            BodyAttribute::VELOCITY, viewNumber, settings, simulationGroup));
    m_forceAverage.reset (
        new ForceAverage (viewNumber, settings, simulationGroup));
}
