/**
 * @file   DisplayForces.cpp
 * @author Dan R. Lipsa
 * @date  8 June 2010
 *
 * Implementation for the DisplayForces class 
 *
 */

#include "Body.h"
#include "DisplayForces.h"
#include "FoamAlongTime.h"
#include "Foam.h"
#include "GLWidget.h"
#include "ViewSettings.h"
#include "OpenGLUtils.h"

void DisplayForces::Display (ViewNumber::Enum viewNumber) const
{
    if (m_glWidget.GetFoamAlongTime ().ForceUsed ())
    {
	glPushAttrib (GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
	glDisable (GL_DEPTH_TEST);
	glLineWidth (m_glWidget.GetHighlightLineWidth ());
	const vector<Force>& forces = m_glWidget.GetCurrentFoam ().GetForces ();
	BOOST_FOREACH (const Force& force, forces)
	    displayForces (viewNumber, force);
	glPopAttrib ();
    }
}

void DisplayForces::displayForces (
    ViewNumber::Enum viewNumber, const Force& force) const
{
    const G3D::AABox& box = 
	m_glWidget.GetFoamAlongTime ().GetFoam (0).
	GetBody (0).GetBoundingBox ();
    float unitForceSize = 
	m_glWidget.GetForceLength () * (box.high () - box.low ()).length ();
    G3D::Vector3 center = force.m_body->GetCenter ();
    if (m_glWidget.GetViewSettings (viewNumber).IsForceNetworkShown ())
	displayForce (
	    m_glWidget.GetHighlightColor (viewNumber, HighlightNumber::H0),
	    center, G3D::Vector3 (unitForceSize * force.m_networkForce, 0));
    if (m_glWidget.GetViewSettings (viewNumber).IsForcePressureShown ())
	displayForce (
	    m_glWidget.GetHighlightColor (viewNumber, HighlightNumber::H1),
	    center, G3D::Vector3 (unitForceSize * force.m_pressureForce, 0));
    if (m_glWidget.GetViewSettings (viewNumber).IsForceResultShown ())
	displayForce (
	    m_glWidget.GetHighlightColor (viewNumber, HighlightNumber::H2),
	    center, G3D::Vector3 (
		unitForceSize * 
		(force.m_networkForce + force.m_pressureForce), 0));
}

void DisplayForces::displayForce (QColor color,
    const G3D::Vector3& center, const G3D::Vector3& force) const
{
    glColor (color);
    glBegin (GL_LINES);
    ::glVertex (center);
    ::glVertex (center + force);
    glEnd ();
}
