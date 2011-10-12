/**
 * @file   ForceAverage.cpp
 * @author Dan R. Lipsa
 * @date  8 June 2010
 *
 * Implementation for the ForceAverage class 
 *
 */

#include "Body.h"
#include "DebugStream.h"
#include "ForceAverage.h"
#include "Simulation.h"
#include "Foam.h"
#include "GLWidget.h"
#include "ViewSettings.h"
#include "OpenGLUtils.h"

void ForceAverage::AverageInit (ViewNumber::Enum viewNumber)
{
    (void)viewNumber;
    Average::AverageInit (viewNumber);
    const vector<Force>& forces = 
	GetGLWidget ().GetSimulation (viewNumber).GetFoam (0).GetForces ();
    m_average.resize (forces.size ());
    for (size_t i = 0; i < forces.size (); ++i)
    {
	m_average[i].m_body = forces[i].m_body;
	m_average[i].m_networkForce = G3D::Vector2::zero ();
	m_average[i].m_pressureForce = G3D::Vector2::zero ();
    }
}

void ForceAverage::addStep (ViewNumber::Enum viewNumber, size_t timeStep, 
			    size_t subStep)
{
    (void)viewNumber;(void)subStep;
    const vector<Force>& forces = 
	GetGLWidget ().GetSimulation (viewNumber).
	GetFoam (timeStep).GetForces ();
    bool forward = (timeStep == GetGLWidget ().GetCurrentTime (viewNumber));
    for (size_t i = 0; i < forces.size (); ++i)
    {
	if (forward)
	    m_average[i].m_body = forces[i].m_body;
	m_average[i].m_networkForce += forces[i].m_networkForce;
	m_average[i].m_pressureForce += forces[i].m_pressureForce;
    }
}

void ForceAverage::removeStep (ViewNumber::Enum viewNumber, size_t timeStep, 
			       size_t subStep)
{
    (void)viewNumber;(void)subStep;
    const vector<Force>& forces = 
	GetGLWidget ().
	GetSimulation (viewNumber).GetFoam (timeStep).GetForces ();
    bool backward = ((timeStep - 1) == 
		     GetGLWidget ().GetCurrentTime (viewNumber));
    for (size_t i = 0; i < forces.size (); ++i)
    {
	if (backward)
	{
	    const vector<Force>& prevForces = 
		GetGLWidget ().GetSimulation (viewNumber).
		GetFoam (timeStep - 1).GetForces ();
	    m_average[i].m_body = prevForces[i].m_body;
	}
	m_average[i].m_networkForce -= forces[i].m_networkForce;
	m_average[i].m_pressureForce -= forces[i].m_pressureForce;
    }
}

void ForceAverage::Display (
    ViewNumber::Enum viewNumber,
    bool adjustForAverageAroundMovementRotation) const
{
    display (viewNumber, m_average, 
	     GetCurrentTimeWindow (), adjustForAverageAroundMovementRotation);
}

void ForceAverage::DisplayOneTimeStep (
    ViewNumber::Enum viewNumber) const
{
    const GLWidget& widgetGl = GetGLWidget ();
    display (
	viewNumber, widgetGl.GetSimulation (viewNumber).
	GetFoam (widgetGl.GetCurrentTime (viewNumber)).GetForces (), 1, false);
}

void ForceAverage::AverageRotateAndDisplay (
    ViewNumber::Enum viewNumber, StatisticsType::Enum displayType,
    G3D::Vector2 rotationCenter, float angleDegrees) const
{
    (void)displayType;
    (void)displayType;(void)rotationCenter;(void)angleDegrees;
    display (viewNumber, m_average, GetCurrentTimeWindow ());
    cdbg << "WARNING: This function show not be used" << endl;
}


void ForceAverage::display (
    ViewNumber::Enum viewNumber, const vector<Force>& forces, size_t count,
    bool adjustForAverageAroundMovementRotation) const
{
    const GLWidget& glWidget = GetGLWidget ();
    if (glWidget.GetSimulation (viewNumber).ForceUsed ())
    {
	glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
	if (adjustForAverageAroundMovementRotation)
	{
	    glMatrixMode (GL_MODELVIEW);
	    glPushMatrix ();
	    glWidget.RotateAndTranslateAverageAround (
		viewNumber, glWidget.GetCurrentTime (viewNumber), -1);
	}
	glDisable (GL_DEPTH_TEST);
	glLineWidth (GetGLWidget ().GetHighlightLineWidth ());
	BOOST_FOREACH (const Force& force, forces)
	    displayForces (viewNumber, force, count);
	if (adjustForAverageAroundMovementRotation)
	    glPopMatrix ();
	glPopAttrib ();
    }
}

void ForceAverage::displayForces (
    ViewNumber::Enum viewNumber, const Force& force, size_t count) const
{
    const G3D::AABox& box = 
	GetGLWidget ().GetSimulation (viewNumber).GetFoam (0).
	GetBody (0).GetBoundingBox ();
    float unitForceSize = 
	GetGLWidget ().GetForceLength () * 
	(box.high () - box.low ()).length () / count;
    G3D::Vector3 center = force.m_body->GetCenter ();
    if (GetGLWidget ().GetViewSettings (viewNumber).IsForceResultShown ())
	displayForce (
	    GetGLWidget ().GetHighlightColor (viewNumber, HighlightNumber::H2),
	    center, G3D::Vector3 (
		unitForceSize * 
		(force.m_networkForce + force.m_pressureForce), 0));
    if (GetGLWidget ().GetViewSettings (viewNumber).IsForceNetworkShown ())
	displayForce (
	    GetGLWidget ().GetHighlightColor (viewNumber, HighlightNumber::H0),
	    center, G3D::Vector3 (unitForceSize * force.m_networkForce, 0));
    if (GetGLWidget ().GetViewSettings (viewNumber).IsForcePressureShown ())
	displayForce (
	    GetGLWidget ().GetHighlightColor (viewNumber, HighlightNumber::H1),
	    center, G3D::Vector3 (unitForceSize * force.m_pressureForce, 0));
}

void ForceAverage::displayForce (QColor color,
    const G3D::Vector3& center, const G3D::Vector3& force) const
{
    glColor (color);
    glBegin (GL_LINES);
    ::glVertex (center);
    ::glVertex (center + force);
    glEnd ();
}
