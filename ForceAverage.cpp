/**
 * @file   ForceAverage.cpp
 * @author Dan R. Lipsa
 * @date  8 June 2010
 *
 * Implementation for the ForceAverage class 
 *
 */

#include "ObjectPosition.h"
#include "Body.h"
#include "Debug.h"
#include "DerivedData.h"
#include "DisplayEdgeFunctors.h"
#include "Foam.h"
#include "ForceAverage.h"
#include "OpenGLUtils.h"
#include "Settings.h"
#include "Simulation.h"
#include "WidgetGl.h"
#include "ViewSettings.h"




void ForceAverage::AverageInit ()
{
    Average::AverageInit ();
    const vector<ForceOneObject>& forces = GetForces (0);
    m_average.resize (forces.size ());
    for (size_t i = 0; i < forces.size (); ++i)
	m_average[i] = ForceOneObject (forces[i].GetBody ());
}

void ForceAverage::addStep (size_t timeStep, size_t subStep)
{
    (void)subStep;
    const vector<ForceOneObject>& forces = GetForces (timeStep);
    bool forward = 
	(timeStep == GetSettings ().GetViewTime ());
    for (size_t i = 0; i < forces.size (); ++i)
    {
	if (forward)
	    m_average[i].SetBody (forces[i].GetBody ());
	m_average[i] += forces[i];
    }
}

void ForceAverage::removeStep (size_t timeStep, size_t subStep)
{
    (void)subStep;
    const vector<ForceOneObject>& forces = GetForces (timeStep);
    bool backward = 
	((timeStep - 1) == GetSettings ().GetViewTime ());
    for (size_t i = 0; i < forces.size (); ++i)
    {
	if (backward)
	{
	    const vector<ForceOneObject>& prevForces = GetForces (timeStep - 1);
	    m_average[i].SetBody (prevForces[i].GetBody ());
	    // bodyId stays the same
	    // m_average[i].m_bodyId = forces[i].m_bodyId;
	}
	m_average[i] -= forces[i];
    }
}


void ForceAverage::DisplayOneTimeStep (WidgetGl* widgetGl) const
{
    displayForceAllObjects (
        widgetGl,
        GetForces (GetSettings ().GetViewTime (GetViewNumber ())), 1, false);
}

void ForceAverage::AverageRotateAndDisplay (
    StatisticsType::Enum displayType,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    (void)displayType;(void)displayType;(void)rotationCenter;(void)angleDegrees;
    bool isAverageAroundRotationShown = 
        GetViewSettings ().IsAverageAroundRotationShown ();
    displayForceAllObjects (0, m_average, 
                            GetCurrentTimeWindow (), 
                            isAverageAroundRotationShown);
}


void ForceAverage::displayForceAllObjects (
    WidgetGl* widgetGl,
    const vector<ForceOneObject>& forces, size_t count,
    bool isAverageAroundRotationShown) const
{
    const Simulation& simulation = GetSimulation ();
    if (simulation.IsForceAvailable ())
    {
        const ViewSettings& vs = GetViewSettings ();
	glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
	if (isAverageAroundRotationShown)
	{
	    glMatrixMode (GL_MODELVIEW);
	    glPushMatrix ();
	    widgetGl->RotateAndTranslateAverageAround (
                *GetDerivedData ().m_objectPositions,
                vs.GetTime (), -1, WidgetGl::DONT_TRANSLATE);
	}
        if (simulation.Is2D ())
            glDisable (GL_DEPTH_TEST);
	if (vs.IsForceShown (ForceType::DIFFERENCE))
	    displayForceTorqueOneObject (widgetGl,
                getForceDifference (forces) / count);
	else
	    BOOST_FOREACH (const ForceOneObject& force, forces)
		displayForceTorqueOneObject (widgetGl, force / count);
	if (isAverageAroundRotationShown)
	    glPopMatrix ();
	glPopAttrib ();
    }
}

ForceOneObject ForceAverage::getForceDifference (
    const vector<ForceOneObject>& forces) const
{
    RuntimeAssert (forces.size () == 2, 
		   "Force difference can be shown for two objects only.");
    ViewSettings& vs = GetSettings ().GetViewSettings (GetViewNumber ());
    size_t index2 = (vs.GetDifferenceBodyId () != 
                     forces[0].GetBody ()->GetId ());
    size_t index1 = ! index2;
    ForceOneObject forceDifference = forces[index2];
    forceDifference -= forces[index1];
    return forceDifference;
}

void ForceAverage::displayForceTorqueOneObject (
    WidgetGl* widgetGl,
    const ForceOneObject& forceOneObject) const
{
    displayForceOneObject (widgetGl, forceOneObject);
    displayTorqueOneObject (widgetGl, forceOneObject);
}

void ForceAverage::displayForceOneObject (
    WidgetGl* widgetGl,
    const ForceOneObject& forceOneObject) const
{
    const ViewSettings& vs = GetViewSettings ();
    const Simulation& simulation = GetSimulation ();
    float bubbleSize = simulation.GetBubbleDiameter ();
    float unitForceSize = vs.GetForceRatio () * bubbleSize;
    G3D::Vector3 center = forceOneObject.GetBody ()->GetCenter ();

    for (size_t i = ForceType::NETWORK; i <= ForceType::RESULT; ++i)
    {
        ForceType::Enum ft = ForceType::Enum (i);
        HighlightNumber::Enum h = HighlightNumber::Enum (i);
	if (vs.IsForceShown (ft))
	    displayForce (
                widgetGl,
                GetSettings ().GetHighlightColor (GetViewNumber (), h), center,
                unitForceSize * forceOneObject.GetForce (ft));
    }
}

void ForceAverage::displayTorqueOneObject (
    WidgetGl* widgetGl,
    const ForceOneObject& forceOneObject) const
{
    ViewNumber::Enum viewNumber = GetViewNumber ();
    const ViewSettings& vs = GetViewSettings ();
    const Simulation& simulation = GetSimulation ();
    G3D::Vector3 center = forceOneObject.GetBody ()->GetCenter ();
    const Foam& foam = GetFoam ();
    float bubbleSize = simulation.GetBubbleDiameter ();
    float unitForceSize = vs.GetForceRatio () * bubbleSize;

    float onePixel = vs.GetOnePixelInObjectSpace ();
    boost::array<G3D::Vector3, 3> displacement = {{
	    G3D::Vector3 (0, 0, 0),
	    G3D::Vector3 (onePixel, onePixel, 0),
	    G3D::Vector3 (-onePixel, -onePixel, 0)}};
    for (size_t i = ForceType::NETWORK; i <= ForceType::RESULT; ++i)
    {
        ForceType::Enum ft = ForceType::Enum (i);
        HighlightNumber::Enum h = HighlightNumber::Enum (i);
	if (vs.IsTorqueShown (ft))
	    displayTorque (widgetGl,
		GetSettings ().GetHighlightColor (viewNumber, h),
		center + displacement[i], 
		vs.GetTorqueDistanceRatio () * bubbleSize,
		foam.GetDmpObjectPosition ().m_angleRadians,
		unitForceSize * forceOneObject.GetTorque (ft));
    }
}

void ForceAverage::displayTorque (
    WidgetGl* widgetGl,
    QColor color, const G3D::Vector3& center, 
    float distance, float angleRadians, float torque) const
{
    pair<G3D::Vector3, G3D::Vector3> centerTorque = 
	calculateTorque (center, distance, angleRadians, torque);
    displayForce (widgetGl, color, centerTorque.first, centerTorque.second);
    glLineWidth (1);
    glBegin (GL_LINES);
    ::glVertex (center);
    ::glVertex (centerTorque.first);
    glEnd ();    
}

pair<G3D::Vector3, G3D::Vector3> ForceAverage::calculateTorque (
    G3D::Vector3 center, float distance, 
    float angleRadians, float torque) const
{
    G3D::Vector2 displacement (0, 1);
    displacement = rotateRadians (displacement, - angleRadians);
    G3D::Vector2 t = rotateRadians (displacement, - M_PI / 2);
    displacement = distance * displacement;
    return pair<G3D::Vector3, G3D::Vector3> (
	G3D::Vector3 (center.xy () + displacement, 0), 
        G3D::Vector3 (t * torque / distance, 0));
}

void ForceAverage::displayForce (
    WidgetGl* widgetGl,
    QColor color, const G3D::Vector3& center, const G3D::Vector3& force) const
{
    const Simulation& simulation = GetSimulation ();
    glColor (color);
    DisplayVtkArrow (center, force,
                     simulation.Is2D () ? 0 : widgetGl->GetQuadric ());
}

const vector<ForceOneObject>& ForceAverage::GetForces (
    size_t timeStep) const
{
    return GetFoam (timeStep).GetForces ();
}

const vector<ForceOneObject>& ForceAverage::GetForces () const
{
    return GetForces (GetTime ());
}


ForceOneObject ForceAverage::GetAverageOneObject (size_t i) const
{
    return m_average[i] / GetCurrentTimeWindow ();
}
