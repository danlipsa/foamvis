/**
 * @file   ViewAverage3D.cpp
 * @author Dan R. Lipsa
 * @date 7 Feb 2013
 *
 * Definitions for the view average
 */

#include "ForceAverage.h"
#include "RegularGridAverage.h"
#include "Settings.h"
#include "ViewAverage3D.h"
#include "ViewSettings.h"


ViewAverage3D::ViewAverage3D (
    ViewNumber::Enum viewNumber,
    const Settings& settings, const SimulationGroup& simulationGroup) :
    AverageInterface (viewNumber),
    m_scalarAverage (
        new RegularGridAverage (
            settings.GetViewSettings (viewNumber).GetBodyOrFaceScalar (),
            viewNumber, settings, simulationGroup)),
    m_velocityAverage (
        new RegularGridAverage (
            BodyAttribute::VELOCITY, viewNumber, settings, simulationGroup)),
    m_deformationAverage (
        new RegularGridAverage (
            BodyAttribute::VELOCITY, viewNumber, settings, simulationGroup)),
    m_forceAverage (new ForceAverage (viewNumber, settings, simulationGroup))
{
}
