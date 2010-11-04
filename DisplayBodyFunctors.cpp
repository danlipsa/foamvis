/**
 * @file   DisplayBodyFunctors.h
 * @author Dan R. Lipsa
 * @date  26 Oct. 2010
 *
 * Implementation for functors to display a body
 */

#include "DisplayBodyFunctors.h"

DisplayBodyCenter::DisplayBodyCenter (
    const GLWidget& widget, const BodySelector& bodySelector, 
    bool useZPos, double zPos):
    
    DisplayBodyBase<> (widget, bodySelector, TexCoordSetter(widget), 
		       BodyProperty::NONE, useZPos, zPos)
{}


void DisplayBodyCenter::display (boost::shared_ptr<Body> b, FocusContext fc)
{
    if (fc == FOCUS)
    {
	glBegin(GL_POINTS);
	G3D::Vector3 v = b->GetCenter ();
	glVertex(G3D::Vector3 (v.xy (), m_zPos));
	glEnd ();
    }
}
