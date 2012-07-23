/**
 * @file   ForceAverage.cpp
 * @author Dan R. Lipsa
 * @date  8 June 2010
 *
 * Implementation for the ForceAverage class 
 *
 */

#include "Body.h"
#include "Debug.h"
#include "DebugStream.h"
#include "DisplayEdgeFunctors.h"
#include "Foam.h"
#include "ForceAverage.h"
#include "OpenGLUtils.h"
#include "Settings.h"
#include "Simulation.h"
#include "WidgetGl.h"
#include "ViewSettings.h"




void ForceAverage::AverageInit (ViewNumber::Enum viewNumber)
{
    (void)viewNumber;
    Average::AverageInit (viewNumber);
    const vector<ForcesOneObject>& forces = GetForces (viewNumber, 0);
    m_average.resize (forces.size ());
    for (size_t i = 0; i < forces.size (); ++i)
	m_average[i] = ForcesOneObject (forces[i].m_bodyId, forces[i].m_body);
}

void ForceAverage::addStep (ViewNumber::Enum viewNumber, size_t timeStep, 
			    size_t subStep)
{
    (void)viewNumber;(void)subStep;
    const vector<ForcesOneObject>& forces = GetForces (viewNumber, timeStep);
    bool forward = (timeStep == GetSettings ().GetCurrentTime (viewNumber));
    for (size_t i = 0; i < forces.size (); ++i)
    {
	if (forward)
	{
	    m_average[i].m_body = forces[i].m_body;
	    // bodyId stays the same
	    //m_average[i].m_bodyId = forces[i].m_bodyId;
	}
	m_average[i] += forces[i];
    }
}

void ForceAverage::removeStep (ViewNumber::Enum viewNumber, size_t timeStep, 
			       size_t subStep)
{
    (void)viewNumber;(void)subStep;
    const vector<ForcesOneObject>& forces = GetForces (viewNumber, timeStep);
    bool backward = ((timeStep - 1) == 
		     GetSettings ().GetCurrentTime (viewNumber));
    for (size_t i = 0; i < forces.size (); ++i)
    {
	if (backward)
	{
	    const vector<ForcesOneObject>& prevForces = GetForces (
		viewNumber, timeStep - 1);
	    m_average[i].m_body = prevForces[i].m_body;
	    // bodyId stays the same
	    // m_average[i].m_bodyId = forces[i].m_bodyId;
	}
	m_average[i] -= forces[i];
    }
}

void ForceAverage::Display (
    ViewNumber::Enum viewNumber,
    bool adjustForAverageAroundMovementRotation) const
{
    displayForcesAllObjects (viewNumber, m_average, 
	     GetCurrentTimeWindow (), adjustForAverageAroundMovementRotation);
}

void ForceAverage::DisplayOneTimeStep (
    ViewNumber::Enum viewNumber) const
{
    displayForcesAllObjects (
	viewNumber, 
	GetForces (
	    viewNumber, GetSettings ().GetCurrentTime (viewNumber)), 1, false);
}

void ForceAverage::AverageRotateAndDisplay (
    ViewNumber::Enum viewNumber, ComputationType::Enum displayType,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    (void)displayType;
    (void)displayType;(void)rotationCenter;(void)angleDegrees;
    displayForcesAllObjects (viewNumber, m_average, GetCurrentTimeWindow ());
    cdbg << "WARNING: This function show not be used" << endl;
}


void ForceAverage::displayForcesAllObjects (
    ViewNumber::Enum viewNumber, 
    const vector<ForcesOneObject>& forces, size_t count,
    bool adjustForAverageAroundMovementRotation) const
{
    const ViewSettings& vs = GetSettings ().GetViewSettings (viewNumber);
    if (GetSimulation (viewNumber).ForcesUsed ())
    {
	glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
	if (adjustForAverageAroundMovementRotation)
	{
	    glMatrixMode (GL_MODELVIEW);
	    glPushMatrix ();
	    vs.RotateAndTranslateAverageAround (vs.GetCurrentTime (), -1);
	}
	glDisable (GL_DEPTH_TEST);
	if (vs.IsForceDifferenceShown ())
	    displayForcesTorqueOneObject (
		viewNumber, getForceDifference (viewNumber, forces), count);
	else
	    BOOST_FOREACH (const ForcesOneObject& force, forces)
		displayForcesTorqueOneObject (viewNumber, force, count);
	if (adjustForAverageAroundMovementRotation)
	    glPopMatrix ();
	glPopAttrib ();
    }
}

const ForcesOneObject ForceAverage::getForceDifference (
    ViewNumber::Enum viewNumber, 
    const vector<ForcesOneObject>& forces) const
{
    RuntimeAssert (forces.size () == 2, 
		   "Force difference can be shown for two objects only.");
    ViewSettings& vs = GetSettings ().GetViewSettings (viewNumber);
    size_t index2 = (vs.GetDifferenceBodyId () != forces[0].m_bodyId);
    size_t index1 = ! index2;
    ForcesOneObject forceDifference = forces[index2];
    forceDifference.m_networkForce = 
	forces[index2].m_networkForce - forces[index1].m_networkForce;
    forceDifference.m_pressureForce = 
	forces[index2].m_pressureForce - forces[index1].m_pressureForce;
    return forceDifference;
}


void ForceAverage::displayForcesTorqueOneObject (
    ViewNumber::Enum viewNumber, const ForcesOneObject& forcesOneObject, 
    size_t count) const
{
    displayForcesOneObject (viewNumber, forcesOneObject, count);
    displayTorqueOneObject (viewNumber, forcesOneObject, count);
}


void ForceAverage::displayForcesOneObject (
    ViewNumber::Enum viewNumber, const ForcesOneObject& forcesOneObject, 
    size_t count) const
{
    ViewSettings& vs = GetSettings ().GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    float bubbleSize = simulation.GetBubbleSize ();
    float unitForceTorqueSize = vs.GetForceTorqueSize () * bubbleSize / count;
    G3D::Vector2 center = forcesOneObject.m_body->GetCenter ().xy ();

    boost::array<bool, 3> isForceShown = {{
	    vs.IsForceNetworkShown (),
	    vs.IsForcePressureShown (),
	    vs.IsForceResultShown ()}};
    boost::array<HighlightNumber::Enum, 3> highlight = {{
	    HighlightNumber::H0,
	    HighlightNumber::H1,
	    HighlightNumber::H2}};
    boost::array<G3D::Vector2, 3> force = {{
	    forcesOneObject.m_networkForce,
	    forcesOneObject.m_pressureForce,
	    forcesOneObject.m_networkForce + forcesOneObject.m_pressureForce}};
    for (size_t i = 0; i < isForceShown.size (); ++i)
	if (isForceShown[i])
	    displayForce (viewNumber,
			  GetSettings ().GetHighlightColor (
			      viewNumber, highlight[i]), center,
			  unitForceTorqueSize * force[i]);    
}




void ForceAverage::displayTorqueOneObject (
    ViewNumber::Enum viewNumber, const ForcesOneObject& forcesOneObject, 
    size_t count) const
{
    ViewSettings& vs = GetSettings ().GetViewSettings (viewNumber);
    const Simulation& simulation = GetSimulation (viewNumber);
    G3D::Vector2 center = forcesOneObject.m_body->GetCenter ().xy ();
    const Foam& foam = GetFoam (viewNumber, 
				GetSettings ().GetCurrentTime (viewNumber));
    float bubbleSize = simulation.GetBubbleSize ();
    float unitForceTorqueSize = vs.GetForceTorqueSize () * bubbleSize / count;

    boost::array<HighlightNumber::Enum, 3> highlight = {{
	    HighlightNumber::H0,
	    HighlightNumber::H1,
	    HighlightNumber::H2}};
    boost::array<bool, 3> isTorqueShown = {{
	    vs.IsTorqueNetworkShown (),
	    vs.IsTorquePressureShown (),
	    vs.IsTorqueResultShown ()}};
    boost::array<float, 3> torque = {{
	    forcesOneObject.m_networkTorque,
	    forcesOneObject.m_pressureTorque,
	    forcesOneObject.m_networkTorque + forcesOneObject.m_pressureTorque}};
    float onePixel = GetOnePixelInObjectSpace ();
    boost::array<G3D::Vector2, 3> displacement = {{
	    G3D::Vector2 (0, 0),
	    G3D::Vector2 (onePixel, onePixel),
	    G3D::Vector2 (-onePixel, -onePixel)}};
    for (size_t i = 0; i < isTorqueShown.size (); ++i)
	if (isTorqueShown[i])
	    displayTorque (
		viewNumber,
		GetSettings ().GetHighlightColor (viewNumber, highlight[i]),
		center.xy () + displacement[i], 
		vs.GetTorqueDistance () * bubbleSize,
		foam.GetDmpObjectPosition ().m_angleRadians,
		unitForceTorqueSize * torque[i]);
}

void ForceAverage::displayTorque (
    ViewNumber::Enum viewNumber, QColor color,
    const G3D::Vector2& center, 
    float distance, float angleRadians, float torque) const
{
    ViewSettings& vs = GetSettings ().GetViewSettings (viewNumber);
    pair<G3D::Vector2, G3D::Vector2> centerTorque = 
	calculateTorque (center, distance, angleRadians, torque);
    displayForce (viewNumber, color,
		  centerTorque.first, centerTorque.second);
    glLineWidth (vs.GetForceTorqueLineWidth ());
    glBegin (GL_LINES);
    ::glVertex (center);
    ::glVertex (centerTorque.first);
    glEnd ();    
}

pair<G3D::Vector2, G3D::Vector2> ForceAverage::calculateTorque (
    G3D::Vector2 center, float distance, 
    float angleRadians, float torque) const
{
    G3D::Vector2 displacement (0, 1);
    displacement = rotateRadians (displacement, - angleRadians);
    G3D::Vector2 t = rotateRadians (displacement, - M_PI / 2);
    displacement = distance * displacement;
    return pair<G3D::Vector2, G3D::Vector2> (
	center + displacement, t * torque / distance);
}

void ForceAverage::displayForce (
    ViewNumber::Enum viewNumber, QColor color,
    const G3D::Vector2& center, const G3D::Vector2& force) const
{
    ViewSettings& vs = GetSettings ().GetViewSettings (viewNumber);
    glColor (color);
    DisplaySegmentArrow (
	center, force, vs.GetForceTorqueLineWidth (),
	GetOnePixelInObjectSpace (), false);
}


const vector<ForcesOneObject>& ForceAverage::GetForces (
    ViewNumber::Enum viewNumber, size_t timeStep) const
{
    return GetFoam (viewNumber, timeStep).GetForces ();
}
